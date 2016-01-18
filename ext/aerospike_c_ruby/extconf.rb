require "mkmf"


HEADER_DIRS = [
  # First search /opt/local for macports
  '/opt/local/include',

  # Then search /usr/local for people that installed from source
  '/usr/local/include',

  # Finally fall back to /usr
  '/usr/include',
]

LIB_DIRS = [
  # First search /opt/local for macports
  '/opt/local/lib',

  # Then search /usr/local for people that installed from source
  '/usr/local/lib',

  # Finally fall back to /usr
  '/usr/lib',
]


$CFLAGS += " -std=c99 -fno-common -fno-strict-aliasing -fPIC"

$LDFLAGS += " -laerospike -lssl -lcrypto -lpthread -pthread -lrt -lm -lz"

$warnflags.gsub!('-Wdeclaration-after-statement', '') if $warnflags

dir_config('aerospike_c_ruby', HEADER_DIRS, LIB_DIRS)

unless find_header("aerospike/aerospike.h")
  abort "aerospike.h missing"
end


create_makefile("aerospike_c_ruby/aerospike_c_ruby")