#ifndef AEROSPIKE_C_RUBY_H
#define AEROSPIKE_C_RUBY_H

#define AEROSPIKE_C_RUBY_DEBUG

#include <ruby.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

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
#include <aerospike/as_hashmap_iterator.h>
#include <aerospike/as_stringmap.h>
#include <aerospike/as_log.h>
#include <aerospike/aerospike_batch.h>
#include <aerospike/aerospike_scan.h>
#include <aerospike/aerospike_query.h>
#include <aerospike/as_ldt.h>
#include <aerospike/as_password.h>

#include <sym.h>
#include <logger.h>


// ---------------------------------------------------
// macros
//
#define RB_FN_ANY() (VALUE(*)(ANYARGS))

#define rb_zero INT2FIX(0)

#define as_val_int_2_val(val) LONG2FIX( as_integer_get( as_integer_fromval(val) ) )         //(int)    as_val * -> VALUE
#define as_val_str_2_val(val) check_and_force_encoding( rb_str_new2( as_string_tostring( as_string_fromval(val))))   //(string) as_val * -> VALUE
#define as_val_dbl_2_val(val) rb_float_new( as_double_get( as_double_fromval(val) ) )       //(double) as_val * -> VALUE
#define rb_ary_len_int(ary) FIX2INT( rb_funcall(ary, rb_intern("length"), 0) )              //(int)    VALUE -> int
#define rb_ary_len_long(ary) FIX2LONG( rb_funcall(ary, rb_intern("length"), 0) )            //(long)   VALUE -> long

#define destroy_query(query) as_query_destroy(query); xfree(query)

#define rb_foreach_ary_int(ary) for(int i = 0; i < rb_ary_len_int(ary); ++i)
#define rb_foreach_ary_long(ary) for(long i = 0; i < rb_ary_len_long(ary); ++i)

#define RB_LLIST_WORAROUND_BIN rb_str_new2("_rblliststat_")
;

#define rb_mJSON() rb_const_get(rb_cObject, rb_intern("JSON"))
#define rb_mMarshal() rb_const_get(rb_cObject, rb_intern("Marshal"))

#define DEFAULT_GEO_JSON_ELSE "GEO_JSON_NULL_VALUE"
#define RB_COORDINATES_STR rb_str_new2("coordinates")
#define RB_POLYGON_STR rb_str_new2("Polygon")
#define RB_POINT_STR rb_str_new2("Point")
#define RB_CIRCLE_STR rb_str_new2("AeroCircle")
;

// ---------------------------------------------------
// definitions
//
void init_aerospike_c_client(VALUE AerospikeC);     // client.c
void init_aerospike_c_key   (VALUE AerospikeC);     // key.c
void init_aerospike_c_record(VALUE AerospikeC);     // record.c
void init_aerospike_c_udf_task(VALUE AerospikeC);   // udf_task.c
void init_aerospike_c_scan_task(VALUE AerospikeC);  // scan_task.c
void init_aerospike_c_query(VALUE AerospikeC);      // query.c
void init_aerospike_c_policy(VALUE AerospikeC);     // policy.c
void init_aerospike_c_llist(VALUE AerospikeC);      // llist.c
void init_aerospike_c_ldt_proxy(VALUE AerospikeC);  // ldt_proxy.c
void init_aerospike_c_exceptions(VALUE AerospikeC); // exceptions.c
void init_aerospike_c_geo_json(VALUE AerospikeC);   // geo_json.c
void init_aerospike_c_password(VALUE AerospikeC);   // password.c

// query_task.c
void init_aerospike_c_query_task(VALUE AerospikeC);
void query_task_deallocate(as_query * query);

// index_task.c
void init_aerospike_c_index_task(VALUE AerospikeC);
void index_task_deallocate(as_index_task * task);

// utils.c

void start_timing(struct timeval * tm);

aerospike *  get_client_struct(VALUE client);
as_key *     get_key_struct(VALUE key);
as_record *  get_record_struct(VALUE rec);
as_geojson * get_geo_json_struct(VALUE rb_geo);

void raise_as_err(as_error err);

VALUE record2hash(as_record * rec);
void  hash2record(VALUE hash, as_record * rec);

as_arraylist * array2as_list(VALUE ary);
VALUE          as_list2array(as_arraylist * list);

char ** rb_array2inputArray(VALUE ary);
void    inputArray_destroy(char ** inputArray);

char ** rb_array2bin_names(VALUE ary);
void    bin_names_destroy(char ** bin_names, long len);

VALUE  value_to_s(VALUE val);
char * value_to_s_cstr(VALUE val);
char * val_inspect(VALUE val);

VALUE bool2rb_bool(bool val);
bool  rb_bool2bool(VALUE val);

VALUE    as_val2rb_val(as_val * value);
as_val * rb_val2as_val(VALUE value);

void as_val_free(as_val * value);

const char * as_val_type_as_str(as_val * val);
const char * rb_val_type_as_str(VALUE value);

as_query * query_obj2as_query(VALUE query_obj);

as_hashmap * hash2as_hashmap(VALUE hash);
VALUE        as_hashmap2hash(as_hashmap * map);

void * rb_policy2as_policy(VALUE rb_policy);
void * get_policy(VALUE options);

VALUE enable_rb_GC();
VALUE disable_rb_GC();

VALUE as_geojson_2_val(as_geojson * geo);

as_integer * rb_copy_as_integer(as_integer * value);
as_integer * rb_copy_as_integer_from_val(as_val * value);
as_double * rb_copy_as_double(as_double * value);
as_double * rb_copy_as_double_from_val(as_val * value);
as_string * rb_copy_as_string(as_string * value);
as_string * rb_copy_as_string_from_val(as_val * value);
as_geojson * rb_copy_as_geojson(as_geojson * value);
as_geojson * rb_copy_as_geojson_from_val(as_val * value);
as_arraylist * rb_copy_as_arraylist(as_arraylist * value);
as_arraylist * rb_copy_as_arraylist_from_val(as_val * value);
as_hashmap * rb_copy_as_hashmap(as_hashmap * value);
as_hashmap * rb_copy_as_hashmap_from_val(as_val * value);
as_val * rb_copy_as_val(as_val * value);

as_record * rb_copy_as_record(as_record * record);

as_operations * rb_operations2as_operations(VALUE operations);
as_bytes * rb_obj_to_as_bytes(VALUE obj);
VALUE as_bytes_to_rb_obj(as_bytes * bytes);

VALUE check_and_force_encoding(VALUE str);

// ---------------------------------------------------
//
// extern variables
//
extern VALUE rb_aero_AerospikeC;
extern VALUE rb_aero_Client;

extern VALUE rb_aero_Record;
extern VALUE rb_aero_Key;
extern VALUE rb_aero_GeoJson;

extern VALUE rb_aero_Logger;

extern VALUE rb_aero_IndexTask;
extern VALUE rb_aero_UdfTask;
extern VALUE rb_aero_ScanTask;
extern VALUE rb_aero_QueryTask;

extern VALUE rb_aero_Query;

extern VALUE rb_aero_Policy;
extern VALUE rb_aero_WritePolicy;
extern VALUE rb_aero_ReadPolicy;
extern VALUE rb_aero_RemovePolicy;
extern VALUE rb_aero_ApplyPolicy;
extern VALUE rb_aero_QueryPolicy;
extern VALUE rb_aero_OperatePolicy;

extern VALUE rb_aero_Llist;
extern VALUE rb_aero_LdtProxy;

extern VALUE rb_aero_AsError;
extern VALUE rb_aero_MemoryError;
extern VALUE rb_aero_ParseError;
extern VALUE rb_aero_OptionError;
extern VALUE rb_aero_PasswordEngine;

extern pthread_mutex_t G_CALLBACK_MUTEX;

#endif // AEROSPIKE_C_RUBY_H