#ifndef AEROSPIKE_C_RUBY_CLIENT_UTILS_H
#define AEROSPIKE_C_RUBY_CLIENT_UTILS_H

#include <aerospike_c_ruby.h>

void client_deallocate(aerospike * as);
VALUE check_with_header(VALUE bins, VALUE options, as_record * rec);
void * get_policy(VALUE options);
void options2config(as_config * config, VALUE options);
void set_priority_options(as_scan * scan, VALUE options);
as_policy_query * get_query_policy(VALUE query_obj);


#endif // AEROSPIKE_C_RUBY_CLIENT_UTILS_H