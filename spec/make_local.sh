#!/bin/bash

cd ext/aerospike_c_ruby
ruby extconf.rb
make
mv aerospike_c_ruby.so ../../lib/aerospike_c_ruby/aerospike_c_ruby.so
make clean
rm Makefile