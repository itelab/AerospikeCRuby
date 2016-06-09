#ifndef AEROSPIKE_C_RUBY_SCAN_H
#define AEROSPIKE_C_RUBY_SCAN_H

VALUE scan_records(int argc, VALUE * argv, VALUE self);
VALUE execute_udf_on_scan(int argc, VALUE * argv, VALUE self);
VALUE background_execute_udf_on_scan(int argc, VALUE * argv, VALUE self);
VALUE scan_each(int argc, VALUE * argv, VALUE self);

#endif // AEROSPIKE_C_RUBY_SCAN_H