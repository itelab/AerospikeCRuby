#include <aerospike_c_ruby.h>

static int foreach_hash2record(VALUE key, VALUE val, VALUE record);
static int foreach_hash2as_hashmap(VALUE key, VALUE val, VALUE map);
static char * key2bin_name(VALUE key);

// ----------------------------------------------------------------------------------
//
// logger methods
//
void log_debug(const char * msg) {
#ifdef AEROSPIKE_C_RUBY_DEBUG
  if ( TYPE(Logger) != T_OBJECT ) {
    return;
  }
  rb_funcall(Logger, rb_intern("debug"), 1, rb_str_new2(msg));
#endif
}

void log_info(const char * msg) {
  if ( TYPE(Logger) != T_OBJECT ) {
    return;
  }
  rb_funcall(Logger, rb_intern("info"), 1, rb_str_new2(msg));
}

void log_warn(const char * msg) {
  if ( TYPE(Logger) != T_OBJECT ) {
    return;
  }
  rb_funcall(Logger, rb_intern("warn"), 1, rb_str_new2(msg));
}

void log_error(const char * msg) {
  if ( TYPE(Logger) != T_OBJECT ) {
    return;
  }
  rb_funcall(Logger, rb_intern("error"), 1, rb_str_new2(msg));
}

void log_fatal(const char * msg) {
  if ( TYPE(Logger) != T_OBJECT ) {
    return;
  }
  rb_funcall(Logger, rb_intern("fatal"), 1, rb_str_new2(msg));
}

// ----------------------------------------------------------------------------------
//
// unwrap Client VALUE struct into aerospike *
//
aerospike * get_client_struct(VALUE client) {
  aerospike * as;
  Data_Get_Struct(rb_iv_get(client, "client"), aerospike, as);
  return as;
}

// ----------------------------------------------------------------------------------
//
// unwrap Key VALUE struct into as_key *
//
as_key * get_key_struct(VALUE key) {
  as_key * k;
  Data_Get_Struct(rb_iv_get(key, "as_key"), as_key, k);
  return k;
}

// ----------------------------------------------------------------------------------
//
// unwrap Record VALUE struct into as_record *
//
as_record * get_record_struct(VALUE rec) {
  as_record * r;
  Data_Get_Struct(rb_iv_get(rec, "rec"), as_record, r);
  return r;
}

// ----------------------------------------------------------------------------------
//
// raise RuntimeError with as_error info
//
void raise_as_error(as_error err) {
  log_fatal(err.message);
  rb_raise(rb_eRuntimeError, "%s -- Error code: %d, at: [%s:%d]", err.message, err.code, err.file, err.line);
}

// ----------------------------------------------------------------------------------
//
// convert as_record to ruby hash
//
VALUE record2hash(as_record * rec) {
  VALUE hash = rb_hash_new();

  as_record_iterator it;
  as_record_iterator_init(&it, rec);

  while ( as_record_iterator_has_next(&it) ) {
    as_bin * bin   = as_record_iterator_next(&it);
    as_val * value = (as_val *) as_bin_get_value(bin);

    VALUE name = rb_str_new2( as_bin_get_name(bin) );

    rb_hash_aset(hash, name, as_val2rb_val(value));
  }

  return hash;
}

// ----------------------------------------------------------------------------------
//
// convert ruby hash and save its value to record
//
void hash2record(VALUE hash, VALUE rec) {
  rb_hash_foreach(hash, foreach_hash2record, rec);
}

// ----------------------------------------------------------------------------------
//
// convert ruby array to as_arraylist
//
as_arraylist * array2as_list(VALUE ary) {
  VALUE tmp;
  as_arraylist * tmp_list;
  as_hashmap * tmp_map;

  int len = rb_ary_len_int(ary);

  as_arraylist * list = as_arraylist_new(len, 1);

  for ( int i = 0; i < len; ++i ) {
    VALUE element = rb_ary_entry(ary, i);

    switch ( TYPE(element) ) {
      case T_NIL:
        log_debug("[Utils][array2as_list] TYPE(element) -> nil");
        rb_raise(rb_eRuntimeError, "[array2as_list] Array value cannot be nil");
        break;

      case T_SYMBOL:
        log_debug("[Utils][array2as_list] TYPE(element) -> symbol");
        tmp = rb_funcall(element, rb_intern("to_s"), 0);
        as_arraylist_append_str(list, StringValueCStr( tmp ));
        break;

      case T_STRING:
        log_debug("[Utils][array2as_list] TYPE(element) -> string");
        as_arraylist_append_str(list, StringValueCStr( element ));
        break;

      case T_FIXNUM:
        log_debug("[Utils][array2as_list] TYPE(element) -> fixnum");
        as_arraylist_append_int64(list, FIX2LONG(element));
        break;

      case T_ARRAY:
        log_debug("[Utils][array2as_list] TYPE(element) -> array");
        tmp_list = array2as_list(element);
        as_arraylist_append_list(list, (as_list *)tmp_list);
        break;

      case T_HASH:
        tmp_map = hash2as_hashmap(element);
        as_arraylist_append_map(list, (as_map *)tmp_map);
        break;

      case T_FLOAT:
        as_arraylist_append_double(list, NUM2DBL(element));
        break;

      default:
        rb_raise(rb_eRuntimeError, "[array2as_list] Unsupported array value type: %s", rb_val_type_as_str(element));
        break;
    }
  }

  return list;
}

// ----------------------------------------------------------------------------------
//
// convert as_list to ruby array
//
VALUE as_list2array(as_arraylist * list) {
  as_arraylist_iterator it;
  as_arraylist_iterator_init(&it, list);

  VALUE ary = rb_ary_new();

  while (as_arraylist_iterator_has_next(&it) ) {
    as_val * value = as_arraylist_iterator_next(&it);
    rb_ary_push(ary, as_val2rb_val(value));
  }

  return ary;
}

// ----------------------------------------------------------------------------------
//
// foreach hash2record
//
static int foreach_hash2record(VALUE key, VALUE val, VALUE record) {
  VALUE tmp;
  as_arraylist * tmp_list;
  as_hashmap * tmp_map;

  as_record * rec;
  Data_Get_Struct(record, as_record, rec);

  switch ( TYPE(val) ) { // set bin_name = val dependent on type
    case T_NIL:
      log_debug("[Utils][foreach_hash2record] TYPE(val) -> nil");
      as_record_set_nil(rec, key2bin_name(key));
      break;

    case T_SYMBOL:
      log_debug("[Utils][foreach_hash2record] TYPE(val) -> symbol");
      tmp = rb_funcall(val, rb_intern("to_s"), 0);
      as_record_set_str(rec, key2bin_name(key), StringValueCStr(tmp));
      break;

    case T_FIXNUM:
      log_debug("[Utils][foreach_hash2record] TYPE(val) -> fixnum");
      as_record_set_int64(rec, key2bin_name(key), FIX2LONG(val));
      break;

    case T_STRING:
      log_debug("[Utils][foreach_hash2record] TYPE(val) -> string");
      as_record_set_str(rec, key2bin_name(key), StringValueCStr(val));
      break;

    case T_ARRAY:
      log_debug("[Utils][foreach_hash2record] TYPE(val) -> array");
      tmp_list = array2as_list(val);
      as_record_set_list(rec, key2bin_name(key), (as_list *)tmp_list);
      break;

    case T_HASH:
      log_debug("[Utils][foreach_hash2record] TYPE(val) -> hash");
      tmp_map = hash2as_hashmap(val);
      as_record_set_map(rec, key2bin_name(key), (as_map *)tmp_map );
      break;

    case T_FLOAT:
      log_debug("[Utils][foreach_hash2record] TYPE(val) -> float");
      as_record_set_double(rec, key2bin_name(key), NUM2DBL(val));
      break;

    default:
      rb_raise(rb_eRuntimeError, "[Utils][foreach_hash2record] Unsupported record value type: %s", rb_val_type_as_str(val));
      break;
  }

  return ST_CONTINUE;
}

// ----------------------------------------------------------------------------------
//
// convert ruby hash key into char *
//
static char * key2bin_name(VALUE key) {
  VALUE tmp;

  switch ( TYPE(key) ) { // get bin name from key
    case T_NIL:
      rb_raise(rb_eRuntimeError, "Record key cannot be nil");
      break;

    case T_STRING:
      return StringValueCStr(key);
      break;

    default:
      tmp = rb_funcall(key, rb_intern("to_s"), 0);
      return StringValueCStr( tmp );
      break;
  }
}

// ----------------------------------------------------------------------------------
//
// free memory method
//
static void map_deallocate(as_hashmap * map) {
  as_hashmap_destroy(map);
}

// ----------------------------------------------------------------------------------
//
// convert ruby hash to as_hashmap
//
as_hashmap * hash2as_hashmap(VALUE hash) {
  long len = rb_ary_len_long(hash);

  as_hashmap * map = as_hashmap_new(len);

  VALUE hmap = Data_Wrap_Struct(Record, NULL, map_deallocate, map);

  rb_hash_foreach(hash, foreach_hash2as_hashmap, hmap);

  return map;
}

// ----------------------------------------------------------------------------------
//
// foreach hash2as_hashmap
//
static int foreach_hash2as_hashmap(VALUE key, VALUE val, VALUE hmap) {
  VALUE tmp;
  as_arraylist * tmp_list;
  as_hashmap * tmp_map;

  char * bin_name = key2bin_name(key);

  as_hashmap * map;
  Data_Get_Struct(hmap, as_hashmap, map);

  switch ( TYPE(val) ) { // set bin_name = val dependent on type
    case T_NIL:
      rb_raise(rb_eRuntimeError, "Hash value cannot be nil");
      break;

    case T_SYMBOL:
      tmp = rb_funcall(val, rb_intern("to_s"), 0);
      as_stringmap_set_str(map, bin_name, StringValueCStr(tmp));
      break;

    case T_FIXNUM:
      as_stringmap_set_int64(map, bin_name, FIX2LONG(val));
      break;

    case T_STRING:
      as_stringmap_set_str(map, bin_name, StringValueCStr(val));
      break;

    case T_ARRAY:
      tmp_list = array2as_list(val);
      as_stringmap_set_list(map, bin_name, (as_list *)tmp_list);
      break;

    case T_HASH:
      tmp_map = hash2as_hashmap(val);
      as_stringmap_set_map(map, bin_name, (as_map *)tmp_map);
      break;

    case T_FLOAT:
      as_stringmap_set_double(map, bin_name, NUM2DBL(val));
      break;

    default:
      rb_raise(rb_eRuntimeError, "Unsupported record value type: %s", rb_val_type_as_str(val));
      break;
  }

  return ST_CONTINUE;

}

// ----------------------------------------------------------------------------------
//
// convert as_hashmap * map into ruby hash
//
VALUE as_hashmap2hash(as_hashmap * map) {
  VALUE hash = rb_hash_new();

  as_hashmap_iterator it;
  as_hashmap_iterator_init(&it, map);

  while ( as_hashmap_iterator_has_next(&it) ) {
    as_pair * val_pair = as_pair_fromval(as_hashmap_iterator_next(&it));

    as_val * key = val_pair->_1;
    as_val * value = val_pair->_2;

    VALUE name = as_val2rb_val(key);
    VALUE val  = as_val2rb_val(value);

    // rb_raise(rb_eRuntimeError, "key: %s, val: %s", val_inspect(name), val_inspect(val));

    rb_hash_aset(hash, name, val);
  }

  as_hashmap_iterator_destroy(&it);

  return hash;
}

// ----------------------------------------------------------------------------------
//
// ruby array to char ** with last element NULL
// remember to destroy allocated memory with inputArray_destroy(inputArray)
//
char ** rb_array2inputArray(VALUE ary) {
  VALUE str_val;
  VALUE element;
  char ** inputArray;
  char * str;

  long len = rb_ary_len_long(ary);

  inputArray = malloc( (len + 1) * sizeof(char *) );

  for ( long i = 0; i < len; ++i ) {
    element = rb_ary_entry(ary, i);

    if ( TYPE(element) != T_STRING ) {
      str_val = rb_funcall(element, rb_intern("to_s"), 0);
      str = StringValueCStr(str_val);
    }
    else {
      str = StringValueCStr(element);
    }

    inputArray[i] = malloc( strlen(str) * sizeof(char *) );
    strcpy(inputArray[i], str);
  }

  inputArray[len] = NULL;

  return inputArray;
}

// ----------------------------------------------------------------------------------
//
// frees inputArray memory
//
void inputArray_destroy(char ** inputArray) {
  for (int i = 0; ; ++i) {
    free(inputArray[i]);

    if ( inputArray[i] == NULL ) break;
  }

  free(inputArray);
}

// ----------------------------------------------------------------------------------
//
// ruby array to char **
// remember to destroy allocated memory with bin_names_destroy(bin_names)
//
char ** rb_array2bin_names(VALUE ary) {
  VALUE str_val;
  VALUE element;
  char ** bin_names;
  char * str;

  long len = rb_ary_len_long(ary);

  bin_names = malloc( (len) * sizeof(char *) );

  for ( long i = 0; i < len; ++i ) {
    element = rb_ary_entry(ary, i);

    if ( TYPE(element) != T_STRING ) {
      str_val = rb_funcall(element, rb_intern("to_s"), 0);
      str = StringValueCStr(str_val);
    }
    else {
      str = StringValueCStr(element);
    }

    bin_names[i] = malloc( strlen(str) * sizeof(char *) );
    strcpy(bin_names[i], str);
  }

  return bin_names;
}

// ----------------------------------------------------------------------------------
//
// frees bin_names memory
//
void bin_names_destroy(char ** bin_names, long len) {
  for (int i = 0; i < len ; ++i) {
    free(bin_names[i]);
  }

  free(bin_names);
}

// ----------------------------------------------------------------------------------
//
// call to_s on val
//
VALUE value_to_s(VALUE val) {
  if ( TYPE(val) == T_STRING ) return val;

  return rb_funcall(val, rb_intern("to_s"), 0);
}

// ----------------------------------------------------------------------------------
//
// convert bool into TrueClass or FalseClass
//
VALUE bool2rb_bool(bool val) {
  if ( val == true ) {
    return Qtrue;
  }
  else if ( val == false ) {
    return Qfalse;
  }

  return Qnil;
}

// ----------------------------------------------------------------------------------
//
// convert TrueClass or FalseClass into bool
//
bool rb_bool2bool(VALUE val) {
  if ( val == Qtrue ) { return true; }
  else if ( val == Qfalse ) { return false; }
  else { rb_raise(rb_eRuntimeError, "[Utils][rb_bool2bool] Cannot convert %s into bool", rb_val_type_as_str(val)); }
}

// ----------------------------------------------------------------------------------
//
// as_val -> VALUE
//
VALUE as_val2rb_val(as_val * value) {
  switch ( as_val_type(value) ) {
    case AS_NIL:
      return Qnil;
      break;

    case AS_INTEGER:
      return as_val_int_2_val(value);
      break;

    case AS_STRING:
      return as_val_str_2_val(value);
      break;

    case AS_LIST:
      return as_list2array(value);
      break;

    case AS_MAP:
      return as_hashmap2hash(value);
      break;

    case AS_DOUBLE:
      return as_val_dbl_2_val(value);
      break;
  }

  rb_raise(rb_eRuntimeError, "[Utils][as_val2rb_val] Unsupported array value type: %s", as_val_type_as_str(value));
}

// ----------------------------------------------------------------------------------
//
// VALUE -> as_val
//
as_val * rb_val2as_val(VALUE value) {
  switch( TYPE(value) ) {
    case T_NIL:
      return NULL;
      break;

    case T_FIXNUM:
      return as_integer_new(FIX2LONG(value));
      break;

    case T_STRING:
      return as_string_new(StringValueCStr(value), false);
      break;

    case T_ARRAY:
      return array2as_list(value);
      break;

    case T_HASH:
      return hash2as_hashmap(value);
      break;

    default:
      rb_raise(rb_eRuntimeError, "[Utils][rb_val2as_val] Unsupported alue type: %s", rb_val_type_as_str(value));
      break;
  }
}

void as_val_free(as_val * value) {
  switch ( as_val_type(value) ) {
    case AS_NIL:
      return;
      break;

    case AS_INTEGER:
      as_integer_destroy( as_integer_fromval(value) );
      break;

    case AS_STRING:
      as_string_destroy( as_string_fromval(value) );
      break;

    case AS_LIST:
      as_list_destroy( as_list_fromval(value) );
      break;

    case AS_MAP:
      as_map_destroy( as_map_fromval(value) );
      break;

    case AS_DOUBLE:
      as_double_destroy( as_double_fromval(value) );
      break;
  }
}

// ----------------------------------------------------------------------------------
//
// call ruby inspect on val, and convert it into char *
//
char * val_inspect(VALUE val) {
  VALUE tmp = rb_funcall(val, rb_intern("inspect"), 0);
  return StringValueCStr(tmp);
}

// ----------------------------------------------------------------------------------
//
// as_val type into char *
//
const char * as_val_type_as_str(as_val * value) {
  switch ( as_val_type(value) ) {
    case AS_UNDEF:
      return "AS_UNDEF";

    case AS_NIL:
      return "AS_NIL";

    case AS_BOOLEAN:
      return "AS_BOOLEAN";

    case AS_INTEGER:
      return "AS_INTEGER";

    case AS_STRING:
      return "AS_STRING";

    case AS_LIST:
      return "AS_LIST";

    case AS_MAP:
      return "AS_MAP";

    case AS_REC:
      return "AS_REC";

    case AS_PAIR:
      return "AS_PAIR";

    case AS_BYTES:
      return "AS_BYTES";

    case AS_DOUBLE:
      return "AS_DOUBLE";

    case AS_GEOJSON:
      return "AS_GEOJSON";

    default:
      return itoa(as_val_type(value));
  }
}

// ----------------------------------------------------------------------------------
//
// VALUE type into char *
//
const char * rb_val_type_as_str(VALUE value) {
  switch ( TYPE(value) ) {
    case T_NIL:
      return "T_NIL";

    case T_OBJECT:
      return "T_OBJECT";

    case T_CLASS:
      return "T_CLASS";

    case T_MODULE:
      return "T_MODULE";

    case T_FLOAT:
      return "T_FLOAT";

    case T_STRING:
      return "T_STRING";

    case T_REGEXP:
      return "T_REGEXP";

    case T_ARRAY:
      return "T_ARRAY";

    case T_HASH:
      return "T_HASH";

    case T_STRUCT:
      return "T_STRUCT";

    case T_BIGNUM:
      return "T_BIGNUM";

    case T_FIXNUM:
      return "T_FIXNUM";

    case T_COMPLEX:
      return "T_COMPLEX";

    case T_RATIONAL:
      return "T_RATIONAL";

    case T_FILE:
      return "T_FILE";

    case T_TRUE:
      return "T_TRUE";

    case T_FALSE:
      return "T_FALSE";

    case T_DATA:
      return "T_DATA";

    case T_SYMBOL:
      return "T_SYMBOL";

    default:
      return itoa(TYPE(value));
  }
}

// ----------------------------------------------------------------------------------
//
// AerospikeC::Query to as_query *
// need to free after usage: destroy_query(query);
//
as_query * query_obj2as_query(VALUE query_obj) {
  VALUE ns   = rb_iv_get(query_obj, "@namespace");
  VALUE set  = rb_iv_get(query_obj, "@set");
  VALUE bins = rb_iv_get(query_obj, "@bins");

  as_query * query = (as_query *) malloc ( sizeof(as_query) ) ;
  as_query_init(query, StringValueCStr(ns), StringValueCStr(set));

  // only one where clause possible in aerospike-c-client v3.1.24
  as_query_where_init(query, 1);

  int len = rb_ary_len_int(bins);
  as_query_select_init(query, len);

  // ----------------
  // select
  for (int i = 0; i < len; ++i) {
    VALUE bin = rb_ary_entry(bins, i);
    as_query_select(query, StringValueCStr(bin));
  }

  VALUE filter      = rb_iv_get(query_obj, "@filter");
  VALUE filter_type = rb_hash_aref(filter, filter_type_sym);
  VALUE query_bin   = rb_hash_aref(filter, bin_sym);

  // ----------------
  // where
  if ( filter_type == eql_sym ) { // eql
    VALUE val        = rb_hash_aref(filter, value_sym);
    VALUE where_type = rb_hash_aref(filter, type_sym);

    if ( where_type == numeric_sym ) {
      as_query_where(query, StringValueCStr(query_bin), as_integer_equals(FIX2LONG(val)) );
    }
    else if ( where_type == string_sym ) {
      as_query_where(query, StringValueCStr(query_bin), as_string_equals(StringValueCStr(val)) );
    }
    else {
      destroy_query(query);
      rb_raise(rb_eRuntimeError, "[Utils][query_obj2as_query] Unsupported eql value type: %s", val_inspect(val));
    }
  }
  else if ( filter_type == range_sym ) { // range
    VALUE min = rb_hash_aref(filter, min_sym);
    VALUE max = rb_hash_aref(filter, max_sym);

    as_query_where(query, StringValueCStr(query_bin), as_integer_range( FIX2LONG(min), FIX2LONG(max) ) );
  }
  else {
    VALUE tmp = rb_hash_aref(filter, filter_type_sym);
    destroy_query(query);
    rb_raise(rb_eRuntimeError, "[Utils][query_obj2as_query] Unsupported filter type: %s", val_inspect(tmp));
  }

  VALUE order_by = rb_iv_get(query_obj, "@order");

  len = rb_ary_len_int(order_by);
  as_query_orderby_inita(query, len);

  // ----------------
  // order
  for (int i = 0; i < len; ++i) {
    VALUE order = rb_ary_entry(order_by, i);

    VALUE order_bin  = rb_hash_aref(order, bin_sym);
    VALUE order_type = rb_hash_aref(order, order_sym);

    as_query_orderby(query, StringValueCStr(order_bin), FIX2INT(order_type));
  }

  return query;
}

// ----------------------------------------------------------------------------------
//
// unwrap AerospikeC::Policy to as_policy and return its pointer
//
void * rb_policy2as_policy(VALUE rb_policy) {
  void * policy;

  VALUE type = rb_iv_get(rb_policy, "@type");

  if ( type == write_sym ) {
    Data_Get_Struct(rb_iv_get(rb_policy, "policy"), as_policy_write, policy);
  }
  else if ( type == read_sym ) {
    Data_Get_Struct(rb_iv_get(rb_policy, "policy"), as_policy_read, policy);
  }
  else if ( type == remove_sym ) {
    Data_Get_Struct(rb_iv_get(rb_policy, "policy"), as_policy_remove, policy);
  }
  else if ( type == apply_sym ) {
    Data_Get_Struct(rb_iv_get(rb_policy, "policy"), as_policy_apply, policy);
  }
  else if ( type == query_sym ) {
    Data_Get_Struct(rb_iv_get(rb_policy, "policy"), as_policy_query, policy);
  }
  else {
    rb_raise(rb_eRuntimeError, "[Utils][rb_policy2as_policy] unknown policy type: %s", val_inspect(type));
  }

  return policy;
}

// ----------------------------------------------------------------------------------
//
// get policy pointer if options policy given
//
void * get_policy(VALUE options) {
  VALUE option_tmp = rb_hash_aref(options, policy_sym);

  if ( rb_funcall(option_tmp, rb_intern("is_a?"), 1, Policy) == Qtrue ) {
    return rb_policy2as_policy(option_tmp);
  }
  else {
    return NULL;
  }
}