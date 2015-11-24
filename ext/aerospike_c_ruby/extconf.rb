require "mkmf"

# PREFIX = "/home/adam/aerospike/aerospike-client-c/target/Linux-x86_64"
# LIB_PATH = "#{PREFIX}/lib"
# INCLUDE_PATH = "#{PREFIX}/include"

# $CFLAGS += " -I#{INCLUDE_PATH}"
$CFLAGS += " -std=c99 -fno-common -fno-strict-aliasing -fPIC"

$LDFLAGS += " /usr/local/lib/libaerospike.so -lssl -lcrypto -lpthread -lrt -lm -lz"

create_makefile("aerospike_c_ruby/aerospike_c_ruby")