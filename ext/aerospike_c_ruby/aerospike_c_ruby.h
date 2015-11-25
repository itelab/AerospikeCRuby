#ifndef AEROSPIKE_C_RUBY_H
#define AEROSPIKE_C_RUBY_H

#define AEROSPIKE_C_RUBY_DEBUG

#include <ruby.h>

#include <stdio.h>
#include <stdlib.h>

#include <aerospike/aerospike.h>
#include <aerospike/aerospike_index.h>
#include <aerospike/aerospike_key.h>
#include <aerospike/aerospike_udf.h>
#include <aerospike/as_bin.h>
#include <aerospike/as_bytes.h>
#include <aerospike/as_error.h>
#include <aerospike/as_config.h>
#include <aerospike/as_key.h>
#include <aerospike/as_operations.h>
#include <aerospike/as_record.h>
#include <aerospike/as_record_iterator.h>
#include <aerospike/as_status.h>
#include <aerospike/as_string.h>
#include <aerospike/as_val.h>
#include <aerospike/as_list.h>
#include <aerospike/as_arraylist.h>
#include <aerospike/as_list_iterator.h>
#include <aerospike/as_map.h>
#include <aerospike/as_hashmap.h>
#include <aerospike/as_stringmap.h>
#include <aerospike/as_log.h>
#include <aerospike/aerospike_batch.h>


// ---------------------------------------------------
// macros
//
#define RB_FN_ANY() (VALUE(*)(ANYARGS))
#define rb_zero INT2FIX(0)

#define with_header_sym ID2SYM(rb_intern("with_header"))
#define ttl_sym         ID2SYM(rb_intern("ttl"))

#define as_val_int_2_val(val) INT2FIX( as_integer_get( as_integer_fromval(val) ) )          //(int)    as_val * -> VALUE
#define as_val_str_2_val(val) rb_str_new2( as_string_tostring( as_string_fromval(value) ) ) //(string) as_val * -> VALUE
#define rb_ary_len_int(ary) FIX2INT( rb_funcall(ary, rb_intern("length"), 0) )              //(int)    VALUE -> int
#define rb_ary_len_long(ary) FIX2LONG( rb_funcall(ary, rb_intern("length"), 0) )            //(long)   VALUE -> long


// ---------------------------------------------------
// definitions
//
void init_aerospike_c_client(VALUE AerospikeC); // client.c
void init_aerospike_c_key   (VALUE AerospikeC); // key.c
void init_aerospike_c_record(VALUE AerospikeC); // record.c

// utils.c
void log_debug (const char * msg);
void log_info  (const char * msg);
void log_warn  (const char * msg);
void log_error (const char * msg);
void log_fatal (const char * msg);

aerospike * get_client_struct(VALUE client);
as_key * get_key_struct(VALUE key);
as_record * get_record_struct(VALUE rec);

void raise_as_err(as_error err);

VALUE record2hash(as_record * rec);
void hash2record(VALUE hash, VALUE rec);
as_arraylist array2as_list(VALUE ary);
VALUE as_list2array(as_arraylist * list);

char ** rb_array2inputArray(VALUE ary);
void inputArray_destroy(char ** inputArray);

char ** rb_array2bin_names(VALUE ary);
void bin_names_destroy(char ** bin_names, long len);

// ---------------------------------------------------
// extern variables
//
extern VALUE Record;
extern VALUE Key;
extern VALUE Client;
extern VALUE Logger;

#endif // AEROSPIKE_C_RUBY_H