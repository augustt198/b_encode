# BEncode

A bencode encoder/decoder.

## Installation

Add this line to your application's Gemfile:

```ruby
gem 'b_encode'
```

Or install it yourself:

```sh
$ gem install b_encode
```

## Usage

#### Encoding

```ruby
"foo".bencode # => "3:foo"
123.bencode   # => "i123e"

["foo", 123].bencode   # => "l3:fooi123ee"
{"foo" => 123}.bencode # => "d3:fooi123ee"
```

#### Decoding
```ruby
BEncode.decode "3:foo" # => "foo"
BEncode.decode "i123e" # => 123

BEncode.decode "l3:fooi123ee" # => ["foo", 123]
BEncode.decode "d3:fooi123ee" # => {"foo" => 123}
```

### Other
Some very informal benchmarks against the [bencode](https://rubygems.org/gems/bencode) gem:
```
                                     user     system      total        real
b_encode: encoding integers      0.120000   0.000000   0.120000 (  0.119217)
b_encode: encoding strings       0.310000   0.000000   0.310000 (  0.306352)
b_encode: encoding lists         0.990000   0.000000   0.990000 (  0.992975)
b_encode: encoding dicts         1.000000   0.000000   1.000000 (  1.001729)
b_encode: decoding integers      0.460000   0.000000   0.460000 (  0.461679)
b_encode: decoding strings       0.570000   0.000000   0.570000 (  0.573589)
b_encode: decoding lists         1.210000   0.000000   1.210000 (  1.211016)
b_encode: decoding hashes        1.590000   0.000000   1.590000 (  1.592065)
                                     user     system      total        real
bencode: encoding integers       0.440000   0.010000   0.450000 (  0.439681)
bencode: encoding strings        0.560000   0.000000   0.560000 (  0.552505)
bencode: encoding lists          3.720000   0.000000   3.720000 (  3.725596)
bencode: encoding dicts          2.880000   0.000000   2.880000 (  2.884355)
bencode: decoding integers       2.460000   0.000000   2.460000 (  2.466726)
bencode: decoding strings        3.380000   0.000000   3.380000 (  3.377338)
bencode: decoding lists         18.130000   0.010000  18.140000 ( 18.150964)
bencode: decoding hashes        20.940000   0.040000  20.980000 ( 20.982961)
```
