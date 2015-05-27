require "mkmf"

$CFLAGS << ' -std=c99 '

create_makefile("b_encode/b_encode")
