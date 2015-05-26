#include "b_encode.h"

VALUE rb_mBEncode;

int digit_count(int n) {
    if (n == 0)
        return 1;

    n = abs(n);
    int c = 0;
    for (; n > 0; c++)
        n /= 10;

    return c;
}

static VALUE
rb_str_bencode_bang(VALUE str) {
    int strlen = RSTRING_LEN(str);
    // length prefix + ":" + str
    int dc     = digit_count(strlen);
    int newlen = dc + 1 + strlen;

    rb_str_resize(str, newlen);

    char *ptr = RSTRING_PTR(str);

    memmove(ptr + dc + 1, ptr, strlen);
    if (strlen == 0) {
        ptr[0] = '0';
    } else {
        int n = strlen;
        for (int i = 0; n > 0; n /= 10, i++)
            ptr[dc - i - 1] = (n % 10) + '0';
    }
    ptr[dc] = ':';

    return str;
}

static VALUE
rb_str_bencode(VALUE str) {
    str = rb_str_dup(str);
    rb_str_bencode_bang(str);
    return str;
}

static VALUE
rb_integer_bencode(VALUE rb_num) {
    long num = NUM2LONG(rb_num);
    int c = 0;
    if (num == 0) {
        c = 1;
    } else {
        if (num < 0)
            c++; // minus sign
        for (int n = labs(num); n > 0; c++)
            n /= 10;
    }

    // "i"<integer>"e"
    int strlen = c + 2;
    VALUE str = rb_str_new(NULL, strlen);
    char *ptr = RSTRING_PTR(str);

    ptr[0] = 'i';
    if (num == 0) {
        ptr[1] = '0';
    } else {
        if (num < 0) {
            ptr[1] = '-';
        }
        int i = 0;
        for (int n = labs(num); n > 0; n /= 10, i++)
            ptr[c - i] = (n % 10) + '0';
    }
    ptr[c + 1] = 'e';

    return str;
}

static VALUE
rb_ary_bencode(VALUE ary) {
    long len = RARRAY_LEN(ary);
    VALUE b_entries[len];

    int total_len = 0;
    ID bencode_method_sym = rb_intern("bencode");
    for (int i = 0; i < len; i++) {
        VALUE val      = rb_ary_entry(ary, i);
        VALUE bencoded = rb_funcall(val, bencode_method_sym, 0);
        total_len      += RSTRING_LEN(bencoded);
        b_entries[i]   = bencoded;
    }

    // "l"<contents>"e"
    VALUE str = rb_str_new(NULL, total_len + 2);
    char *ptr = RSTRING_PTR(str);

    ptr[0] = 'l';
    
    int offset = 1;
    for (int i = 0; i < len; i++) {
        VALUE entry = b_entries[i];
        int e_len   = RSTRING_LEN(entry);
        memcpy(ptr + offset, RSTRING_PTR(entry), e_len);
        offset += e_len;
    }
    
    ptr[total_len + 1] = 'e';

    return str;
}

struct foreach_info {
    int *idx_ptr;
    VALUE *base;
};

static int
rb_hash_bencode_entry(VALUE key, VALUE value, VALUE arg) {
    if (!RB_TYPE_P(key, T_STRING))
        rb_raise(rb_eTypeError, "All hash keys must be strings");

    VALUE key_bencode = rb_str_bencode(key);

    ID bencode_method_sym = rb_intern("bencode");
    VALUE val_bencode     = rb_funcall(value, bencode_method_sym, 0);

    struct foreach_info *info = (struct foreach_info*) arg;

    int idx     = *(info->idx_ptr);
    VALUE *base = info->base;

    *((VALUE*) base + idx)     = key_bencode;
    *((VALUE*) base + idx + 1) = val_bencode;

    *info->idx_ptr += 2;

    return ST_CONTINUE;
}

static VALUE
rb_hash_bencode(VALUE hash) {
    int hash_size = RHASH_SIZE(hash);
    VALUE pieces[hash_size * 2];
    
    int idx = 0;
    struct foreach_info info = {&idx, pieces};

    rb_hash_foreach(hash, rb_hash_bencode_entry, (VALUE) &info);

    int total_len = 0;
    for (int i = 0; i < hash_size * 2; i++) {
        total_len += RSTRING_LEN(pieces[i]);
    }

    VALUE str = rb_str_new(NULL, total_len + 2);
    char *ptr = RSTRING_PTR(str);

    ptr[0] = 'd';

    int offset = 1;
    for (int i = 0; i < hash_size * 2; i++) {
        int entry_len = RSTRING_LEN(pieces[i]);
        memcpy(ptr + offset, RSTRING_PTR(pieces[i]), entry_len);
        offset += entry_len;
    }

    ptr[total_len + 1] = 'e';

    return str;
}

#define EOF_CHECK(ptr, len)\
    if (*(ptr) >= (len))\
        rb_raise(rb_eIOError, "Unexpected EOF");

VALUE decode_any(char *str, int *idx_ptr, int len);

VALUE decode_integer(char *str, int *idx_ptr, int len) {
    int strlen = 2; // "i"<integer>"e"

    EOF_CHECK(idx_ptr, len);
    int neg = 0; // bool
    if (str[*idx_ptr] == '-') {
        neg = 1;
        
        strlen++;
        *idx_ptr += 1;
        EOF_CHECK(idx_ptr, len);
    }

    long num = 0;
    while (rb_isdigit(str[*idx_ptr])) {
        num = num * 10 + (str[*idx_ptr] - '0');
        strlen++;
        
        *idx_ptr += 1;
        EOF_CHECK(idx_ptr, len);
    }

    if (neg)
        num = -num;

    if (str[*idx_ptr] != 'e')
        rb_raise(rb_eRuntimeError, "Expected 'e'");
    *idx_ptr += 1;

    return LONG2NUM(num);
}

VALUE decode_string(char *str, int *idx_ptr, int len, char dig) {
    EOF_CHECK(idx_ptr, len);
    int strlen = dig - '0';

    while (rb_isdigit(str[*idx_ptr])) {
        strlen = strlen * 10 + (str[*idx_ptr] - '0');
        strlen++;
        *idx_ptr += 1;
        EOF_CHECK(idx_ptr, len);
    }

    if (str[*idx_ptr] != ':')
        rb_raise(rb_eRuntimeError, "Expected ':', got '%c'", str[*idx_ptr]);
    *idx_ptr += 1;

    if (*idx_ptr + strlen > len)
        rb_raise(rb_eIOError, "Unexpected EOF");

    VALUE rb_str = rb_str_new(NULL, strlen);

    memcpy(RSTRING_PTR(rb_str), str + *idx_ptr, strlen);
    *idx_ptr += strlen;

    return rb_str;
}

VALUE decode_list(char *str, int *idx_ptr, int len) {
    EOF_CHECK(idx_ptr, len);

    VALUE ary = rb_ary_new();

    while (str[*idx_ptr] != 'e') {
        VALUE elem = decode_any(str, idx_ptr, len);
        rb_ary_push(ary, elem);
        EOF_CHECK(idx_ptr, len);
    }
    *idx_ptr += 1;

    return ary;
}

VALUE decode_dict(char *str, int *idx_ptr, int len) {
    EOF_CHECK(idx_ptr, len);

    VALUE hash = rb_hash_new();

    while (str[*idx_ptr] != 'e') {
        VALUE key = decode_any(str, idx_ptr, len);
        EOF_CHECK(idx_ptr, len);
        VALUE val = decode_any(str, idx_ptr, len);
        EOF_CHECK(idx_ptr, len);

        rb_hash_aset(hash, key, val);
    }
    *idx_ptr += 1;

    return hash;
}

VALUE decode_any(char *str, int *idx_ptr, int len) {
    EOF_CHECK(idx_ptr, len);
    char prefix = str[*idx_ptr];
    *idx_ptr += 1;

    switch (prefix) {
        case 'i':
            return decode_integer(str, idx_ptr, len);
        case 'l':
            return decode_list(str, idx_ptr, len);
        case 'd':
            return decode_dict(str, idx_ptr, len);
        default:
            if (rb_isdigit(prefix)) {
                return decode_string(str, idx_ptr, len, prefix);
            }
        rb_raise(rb_eRuntimeError, "Unexpected character '%c'", prefix);            
    }

    return Qnil;
}

VALUE rb_bencode_decode(VALUE mod, VALUE str) {
    int idx = 0;

    return decode_any(RSTRING_PTR(str), &idx, RSTRING_LEN(str));
}

void Init_b_encode(void) {
    rb_mBEncode = rb_define_module("BEncode");
    
    // String#bencode and String#bencode!
    rb_define_method(rb_cString, "bencode", rb_str_bencode, 0);
    rb_define_method(rb_cString, "bencode!", rb_str_bencode_bang, 0);
    
    // Integer#bencode
    rb_define_method(rb_cInteger, "bencode", rb_integer_bencode, 0);
    
    // Array#bencode
    rb_define_method(rb_cArray, "bencode", rb_ary_bencode, 0);

    // Hash#bencode
    rb_define_method(rb_cHash, "bencode", rb_hash_bencode, 0);

    rb_define_singleton_method(rb_mBEncode, "decode", rb_bencode_decode, 1);
}
