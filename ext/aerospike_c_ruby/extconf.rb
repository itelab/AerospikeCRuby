require "mkmf"

$CFLAGS += " -std=c99 -fno-common -fno-strict-aliasing -fPIC"

$LDFLAGS += " /usr/local/lib/libaerospike.so -lssl -lcrypto -lpthread -lrt -lm -lz"

$warnflags.gsub!('-Wdeclaration-after-statement', '') if $warnflags

create_makefile("aerospike_c_ruby/aerospike_c_ruby")