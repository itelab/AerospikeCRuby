#ifndef AEROSPIKE_C_RUBY_QUERYING_H
#define AEROSPIKE_C_RUBY_QUERYING_H

VALUE execute_query(VALUE self, VALUE query_obj);
VALUE execute_udf_on_query(int argc, VALUE * argv, VALUE self);
VALUE background_execute_udf_on_query(int argc, VALUE * argv, VALUE self);
VALUE execute_query_each(VALUE self, VALUE query_obj);

#endif // AEROSPIKE_C_RUBY_QUERYING_H