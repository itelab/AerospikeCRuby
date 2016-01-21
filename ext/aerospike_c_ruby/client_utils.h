#ifndef AEROSPIKE_C_RUBY_CLIENT_UTILS_H
#define AEROSPIKE_C_RUBY_CLIENT_UTILS_H

#include <aerospike_c_ruby.h>




typedef struct query_method_options_s {
  aerospike *                       as;
  as_query *                        query;
  as_policy_query *                 policy;
  aerospike_query_foreach_callback  callback;
  as_arraylist *                    args;
  VALUE                             query_data;
} query_method_options;

typedef struct query_item_s {
  struct query_item_s * next;
  struct query_item_s * lastsaved;
  as_record *           rec;
  as_val *              val;
} query_item;

typedef struct query_list_s {
  aerospike *                       as;
  as_query *                        query;
  as_policy_query *                 policy;
  aerospike_query_foreach_callback  callback;
  as_arraylist *                    args;
  query_item *                      query_data;
  VALUE                             result;
} query_list;

typedef struct scan_method_options_s {
  aerospike *                     as;
  as_scan *                       scan;
  as_policy_scan *                policy;
  aerospike_scan_foreach_callback callback;
  as_arraylist *                  args;
  VALUE                           scan_data;
} scan_method_options;

void client_deallocate(aerospike * as);
void options2config(as_config * config, VALUE options, VALUE self);
void set_priority_options(as_scan * scan, VALUE options);
void check_for_llist_workaround(VALUE self, VALUE key, VALUE hash);

VALUE check_with_header(VALUE bins, VALUE options, as_record * rec);

as_policy_query * get_query_policy(VALUE query_obj);
void init_query_item(query_item * item);
void set_query_item_next(query_item * item, query_item * new_item);
void set_query_result_and_destroy(query_list * args);
void query_result_destroy(query_list * args);

#endif // AEROSPIKE_C_RUBY_CLIENT_UTILS_H