#ifndef AEROSPIKE_C_RUBY_H
#define AEROSPIKE_C_RUBY_H

#define AEROSPIKE_C_RUBY_DEBUG

#include <ruby.h>
#include <ruby/thread.h>

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
#include <aerospike/as_predexp.h>

#include <sym.h>
#include <logger.h>
#include <utils.h>


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

#define rb_aero_KEY_INFO rb_funcall(key, rb_intern("key_info"), 0)
#define rb_aero_CLIENT get_client_struct(self)
#define rb_aero_OPERATION rb_const_get(rb_aero_AerospikeC, rb_intern("Operation"))
#define rb_aero_MOD_INFO rb_sprintf("mod: %s, func: %s", c_module_name, c_func_name)
#define rb_aero_KEY get_key_struct(key)
#define rb_aero_TIMED(val) struct timeval val; start_timing(&val)

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
void init_aerospike_c_predexp(VALUE AerospikeC);    // predexp.c

// query_task.c
void init_aerospike_c_query_task(VALUE AerospikeC);
void query_task_deallocate(as_query * query);

// index_task.c
void init_aerospike_c_index_task(VALUE AerospikeC);
void index_task_deallocate(as_index_task * task);

// ---------------------------------------------------
//
// extern variables
//
extern VALUE rb_aero_AerospikeC;
extern VALUE rb_aero_Client;

extern VALUE rb_aero_Record;
extern VALUE rb_aero_Key;
extern VALUE rb_aero_GeoJson;

extern VALUE rb_aero_IndexTask;
extern VALUE rb_aero_UdfTask;
extern VALUE rb_aero_ScanTask;
extern VALUE rb_aero_QueryTask;

extern VALUE rb_aero_Query;
extern VALUE rb_aero_PredExp;

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
