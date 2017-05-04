#ifndef AEROSPIKE_C_RUBY_AS_PREDEXP_ARRAY_H
#define AEROSPIKE_C_RUBY_AS_PREDEXP_ARRAY_H

#include <aerospike_c_ruby.h>

typedef struct as_predexp_array{
  as_predexp_base ** array;
  int capacity;
  int size;
}as_predexp_array;

void init_as_predexp_array(as_predexp_array *a, int initialSize);
void insert_as_predexp_array(as_predexp_array *a, as_predexp_base * element);
void free_as_predexp_array(as_predexp_array *a);
void predexp_node_2_as_predexp(as_predexp_array *a, VALUE node);
void push_2_as_predexp(as_predexp_array *a, VALUE node_bin, VALUE node_true, VALUE node_filters);
void check_hash(VALUE hash);
void check_array(VALUE array);
void raise_parse_error();

#endif // AEROSPIKE_C_RUBY_AS_PREDEXP_ARRAY_H
