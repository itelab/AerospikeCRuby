#ifndef AEROSPIKE_C_RUBY_CLIENT_UTILS_H
#define AEROSPIKE_C_RUBY_CLIENT_UTILS_H

#include <aerospike_c_ruby.h>

void client_deallocate(aerospike * as);
VALUE check_with_header(VALUE bins, VALUE options, as_record * rec);
void options2config(as_config * config, VALUE options, VALUE self);
void set_priority_options(as_scan * scan, VALUE options);
as_policy_query * get_query_policy(VALUE query_obj);
void check_for_llist_workaround(VALUE self, VALUE key, VALUE hash);

#endif // AEROSPIKE_C_RUBY_CLIENT_UTILS_H