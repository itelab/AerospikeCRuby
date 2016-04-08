#include <aerospike_c_ruby.h>

static int foreach_hash2record(VALUE key, VALUE val, VALUE record);
static int foreach_hash2as_hashmap(VALUE key, VALUE val, VALUE map);
static char * key2bin_name(VALUE key);

static int color_code = 35;

static void switch_color_code() {
  if (color_code == 35) color_code = 36;
  else if (color_code == 36) color_code = 35;
  else color_code = 36;
}

// ----------------------------------------------------------------------------------
//
// logger methods
//
void log_debug(const char * msg) {
#ifdef AEROSPIKE_C_RUBY_DEBUG
  if ( TYPE(rb_aero_Logger) != T_OBJECT ) return;

  switch_color_code();

  VALUE rb_msg = rb_sprintf("\e[1m\e[%dm%s\e[0m %s", color_code, "<AerospikeC>", msg);
  rb_funcall(rb_aero_Logger, rb_intern("debug"), 1, rb_msg);
#endif
}

void log_info(const char * msg) {
  if ( TYPE(rb_aero_Logger) != T_OBJECT ) return;

  switch_color_code();

  VALUE rb_msg = rb_sprintf("\e[1m\e[%dm%s\e[0m %s", color_code, "<AerospikeC>", msg);
  rb_funcall(rb_aero_Logger, rb_intern("info"), 1, rb_msg);
}

void log_warn(const char * msg) {
  if ( TYPE(rb_aero_Logger) != T_OBJECT ) return;

  switch_color_code();

  VALUE rb_msg = rb_sprintf("\e[1m\e[%dm%s\e[0m %s", color_code, "<AerospikeC>", msg);
  rb_funcall(rb_aero_Logger, rb_intern("warn"), 1, rb_msg);
}

void log_error(const char * msg) {
  if ( TYPE(rb_aero_Logger) != T_OBJECT ) return;

  switch_color_code();

  VALUE rb_msg = rb_sprintf("\e[1m\e[%dm%s\e[0m \e[1m\e[31%s\e[0m", color_code, "<AerospikeC>", msg);
  rb_funcall(rb_aero_Logger, rb_intern("error"), 1, rb_msg);
}

void log_fatal(const char * msg) {
  if ( TYPE(rb_aero_Logger) != T_OBJECT ) return;

  switch_color_code();

  VALUE rb_msg = rb_sprintf("\e[1m\e[%dm%s\e[0m \e[1m\e[31%s\e[0m", color_code, "<AerospikeC>", msg);
  rb_funcall(rb_aero_Logger, rb_intern("fatal"), 1, rb_msg);
}

void log_info_with_time(const char * msg, struct timeval * tm) {
  if ( TYPE(rb_aero_Logger) != T_OBJECT ) return;

  struct timeval tm2;
  gettimeofday(&tm2, NULL);

  double elapsedTime = (tm2.tv_sec - tm->tv_sec) * 1000.0;
  elapsedTime += (tm2.tv_usec - tm->tv_usec) / 1000.0;

  switch_color_code();

  VALUE rb_msg = rb_sprintf("\e[1m\e[%dm%s (%.4f ms)\e[0m \e[1m%s\e[0m", color_code, "<AerospikeC>", elapsedTime, msg);
  rb_funcall(rb_aero_Logger, rb_intern("info"), 1, rb_msg);
}

void log_info_with_time_v(const char * msg, struct timeval * tm, VALUE val) {
  if ( TYPE(rb_aero_Logger) != T_OBJECT ) return;

  struct timeval tm2;
  gettimeofday(&tm2, NULL);

  double elapsedTime = (tm2.tv_sec - tm->tv_sec) * 1000.0;
  elapsedTime += (tm2.tv_usec - tm->tv_usec) / 1000.0;

  switch_color_code();

  VALUE rb_msg = rb_sprintf("\e[1m\e[%dm%s (%.4f ms)\e[0m \e[1m%s, %"PRIsVALUE"\e[0m", color_code, "<AerospikeC>", elapsedTime, msg, val);
  rb_funcall(rb_aero_Logger, rb_intern("info"), 1, rb_msg);
}

void log_info_with_time_v2(const char * msg, struct timeval * tm, VALUE val, VALUE val2) {
  if ( TYPE(rb_aero_Logger) != T_OBJECT ) return;

  struct timeval tm2;
  gettimeofday(&tm2, NULL);

  double elapsedTime = (tm2.tv_sec - tm->tv_sec) * 1000.0;
  elapsedTime += (tm2.tv_usec - tm->tv_usec) / 1000.0;

  switch_color_code();

  VALUE rb_msg = rb_sprintf("\e[1m\e[%dm%s (%.4f ms)\e[0m \e[1m%s, %"PRIsVALUE", %"PRIsVALUE"\e[0m", color_code, "<AerospikeC>", elapsedTime, msg, val, val2);
  rb_funcall(rb_aero_Logger, rb_intern("info"), 1, rb_msg);
}

void start_timing(struct timeval * tm) {
  gettimeofday(tm, NULL);
}


// ----------------------------------------------------------------------------------
//
// unwrap Client VALUE struct into aerospike *
//
aerospike * get_client_struct(VALUE client) {
  aerospike * as;
  Data_Get_Struct(client, aerospike, as);
  return as;
}

// ----------------------------------------------------------------------------------
//
// unwrap Key VALUE struct into as_key *
//
as_key * get_key_struct(VALUE key) {
  as_key * k;
  Data_Get_Struct(key, as_key, k);
  return k;
}

// ----------------------------------------------------------------------------------
//
// unwrap Record VALUE struct into as_record *
//
as_record * get_record_struct(VALUE rec) {
  as_record * r;
  Data_Get_Struct(rec, as_record, r);
  return r;
}

// ----------------------------------------------------------------------------------
//
// unwrap GeoJson VALUE struct into as_geojson *
//
as_geojson * get_geo_json_struct(VALUE rb_geo) {
  as_geojson * geo;
  Data_Get_Struct(rb_geo, as_geojson, geo);
  return geo;
}

// ----------------------------------------------------------------------------------
//
// raise RuntimeError with as_error info
//
void raise_as_error(as_error err) {
  log_fatal(err.message);
  rb_raise(rb_aero_AsError, "%s -- Error code: %d, at: [%s:%d]", err.message, err.code, err.file, err.line);
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
  // log_debug("Converted ruby hash into as_record");
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
        rb_raise(rb_aero_ParseError, "[array2as_list] Array value cannot be nil");
        break;

      case T_SYMBOL:
        tmp = rb_funcall(element, rb_intern("to_s"), 0);
        as_arraylist_append_str(list, StringValueCStr( tmp ));
        break;

      case T_STRING:
        as_arraylist_append_str(list, StringValueCStr( element ));
        break;

      case T_FIXNUM:
        as_arraylist_append_int64(list, FIX2LONG(element));
        break;

      case T_ARRAY:
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
        rb_raise(rb_aero_ParseError, "[array2as_list] Unsupported array value type: %s", rb_val_type_as_str(element));
        break;
    }
  }

  // log_debug("Converted ruby array into as_list");

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

  // log_debug("Converted as_list into ruby array");

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
      as_record_set_nil(rec, key2bin_name(key));
      break;

    case T_SYMBOL:
      tmp = rb_funcall(val, rb_intern("to_s"), 0);
      as_record_set_str(rec, key2bin_name(key), StringValueCStr(tmp));
      break;

    case T_FIXNUM:
      as_record_set_int64(rec, key2bin_name(key), FIX2LONG(val));
      break;

    case T_STRING:
      as_record_set_str(rec, key2bin_name(key), StringValueCStr(val));
      break;

    case T_ARRAY:
      tmp_list = array2as_list(val);
      as_record_set_list(rec, key2bin_name(key), (as_list *)tmp_list);
      break;

    case T_HASH:
      tmp_map = hash2as_hashmap(val);
      as_record_set_map(rec, key2bin_name(key), (as_map *)tmp_map );
      break;

    case T_FLOAT:
      as_record_set_double(rec, key2bin_name(key), NUM2DBL(val));
      break;

    case T_DATA:
      if ( rb_funcall(val, rb_intern("is_a?"), 1, rb_aero_GeoJson) == Qtrue ) {
        as_record_set_geojson(rec, key2bin_name(key), get_geo_json_struct(val));
      }

      break;

    default:
      rb_raise(rb_aero_ParseError, "[Utils][foreach_hash2record] Unsupported record value type: %s", rb_val_type_as_str(val));
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
      rb_raise(rb_aero_ParseError, "Record key cannot be nil: %s", val_inspect(key));
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

  VALUE hmap = Data_Wrap_Struct(rb_aero_Record, NULL, map_deallocate, map);

  rb_hash_foreach(hash, foreach_hash2as_hashmap, hmap);

  // log_debug("Converted ruby hash into as_hashmap");

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
      rb_raise(rb_aero_ParseError, "Hash value cannot be nil: %s", val_inspect(val));
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
      rb_raise(rb_aero_ParseError, "Unsupported record value type: %s", rb_val_type_as_str(val));
      break;
  }

  return ST_CONTINUE;

}

// ----------------------------------------------------------------------------------
//
// convert as_hashmap * map into ruby hash
//
VALUE as_hashmap2hash(as_hashmap * map) {
  VALUE name;
  VALUE val;
  VALUE hash = rb_hash_new();

  as_hashmap_iterator it;
  as_hashmap_iterator_init(&it, map);

  while ( as_hashmap_iterator_has_next(&it) ) {
    as_pair * val_pair = as_pair_fromval(as_hashmap_iterator_next(&it));

    as_val * key = val_pair->_1;
    as_val * value = val_pair->_2;

    as_string * i = as_string_fromval(key);

    name = rb_str_new2(i->value);
    val  = as_val2rb_val(value);

    rb_hash_aset(hash, name, val);
  }

  as_hashmap_iterator_destroy(&it);

  // log_debug("Converted as_hashmap into ruby hash");

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
// call to_s on val and retrun as c string
//
char * value_to_s_cstr(VALUE val) {
  VALUE tmp = rb_funcall(val, rb_intern("to_s"), 0);

  return StringValueCStr(tmp);
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
  else { rb_raise(rb_aero_ParseError, "[Utils][rb_bool2bool] Cannot convert %s into bool", rb_val_type_as_str(val)); }
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

    case AS_GEOJSON:
      return as_geojson_2_val(value);
      break;

    case AS_UNDEF:
      return rb_str_new2("undef");
      break;
  }

  return rb_str_new2(as_val_type_as_str(value));
}

// ----------------------------------------------------------------------------------
//
// VALUE -> as_val
//
as_val * rb_val2as_val(VALUE value) {
  switch( TYPE(value) ) {
    case T_NIL:
      return NULL;

    case T_FIXNUM:
      return as_integer_new(FIX2LONG(value));

    case T_FLOAT:
      return as_double_new(NUM2DBL(value));

    case T_STRING:
      return as_string_new(StringValueCStr(value), false);

    case T_ARRAY:
      return array2as_list(value);

    case T_HASH:
      return hash2as_hashmap(value);

    default:
      rb_raise(rb_aero_ParseError, "[Utils][rb_val2as_val] Unsupported alue type: %s", rb_val_type_as_str(value));
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

  as_query * query = as_query_new(StringValueCStr(ns), StringValueCStr(set));
  // as_query_init(query, StringValueCStr(ns), StringValueCStr(set));

  // only one where clause possible in aerospike-c-client v3.1.24
  as_query_where_init(query, 1);

  int len = rb_ary_len_int(bins);

  if ( len > 0 ) {
    as_query_select_init(query, len);

    // ----------------
    // select
    for (int i = 0; i < len; ++i) {
      VALUE bin = rb_ary_entry(bins, i);
      as_query_select(query, StringValueCStr(bin));
    }
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
      rb_raise(rb_aero_ParseError, "[Utils][query_obj2as_query] Unsupported eql value type: %s", val_inspect(val));
    }
  }
  else if ( filter_type == range_sym ) { // range
    VALUE min = rb_hash_aref(filter, min_sym);
    VALUE max = rb_hash_aref(filter, max_sym);

    as_query_where(query, StringValueCStr(query_bin), as_integer_range( FIX2LONG(min), FIX2LONG(max) ) );
  }
  else if ( filter_type == geo_within_sym ) { // geo_within
    VALUE val = rb_hash_aref(filter, value_sym);

    as_geojson * geo = get_geo_json_struct(val);

    char * buffer = (char *) malloc ( strlen(geo->value) + 1 );
    strcpy(buffer, geo->value);

    as_query_where(query, StringValueCStr(query_bin), as_geo_within(buffer));
  }
  else if ( filter_type == geo_contains_sym ) { // geo_contains
    VALUE val = rb_hash_aref(filter, value_sym);

    as_geojson * geo = get_geo_json_struct(val);

    char * buffer = (char *) malloc ( strlen(geo->value) + 1 );
    strcpy(buffer, geo->value);

    as_query_where(query, StringValueCStr(query_bin), as_geo_contains(buffer));
  }
  else {
    VALUE tmp = rb_hash_aref(filter, filter_type_sym);
    destroy_query(query);
    rb_raise(rb_aero_ParseError, "[Utils][query_obj2as_query] Unsupported filter type: %s", val_inspect(tmp));
  }

  VALUE order_by = rb_iv_get(query_obj, "@order");

  len = rb_ary_len_int(order_by);

  if ( len > 0 ) {
    as_query_orderby_inita(query, len);

    // ----------------
    // order
    for (int i = 0; i < len; ++i) {
      VALUE order = rb_ary_entry(order_by, i);

      VALUE order_bin  = rb_hash_aref(order, bin_sym);
      VALUE order_type = rb_hash_aref(order, order_sym);

      as_query_orderby(query, StringValueCStr(order_bin), FIX2INT(order_type));
    }
  }

  // log_debug("Converted ruby AerospikeC::Query to as_query");

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
  else if ( type == operate_sym ) {
    Data_Get_Struct(rb_iv_get(rb_policy, "policy"), as_policy_operate, policy);
  }
  else {
    rb_raise(rb_aero_ParseError, "[Utils][rb_policy2as_policy] unknown policy type: %s", val_inspect(type));
  }

  // log_debug("Converted ruby AerospikeC::Policy into as_policy");

  return policy;
}

// ----------------------------------------------------------------------------------
//
// get policy pointer if options policy given
//
void * get_policy(VALUE options) {
  VALUE option_tmp = rb_hash_aref(options, policy_sym);

  if ( rb_funcall(option_tmp, rb_intern("is_a?"), 1, rb_aero_Policy) == Qtrue ) {
    return rb_policy2as_policy(option_tmp);
  }
  else {
    return NULL;
  }
}

// ----------------------------------------------------------------------------------
//
// enable ruby garbage collector
//
VALUE enable_rb_GC() {
  log_debug("Enabling ruby GC");
  return rb_funcall(rb_mGC, rb_intern("enable"), 0);
}

// ----------------------------------------------------------------------------------
//
// disable ruby garbage collector
//
VALUE disable_rb_GC() {
  log_debug("Disabling ruby GC");
  return rb_funcall(rb_mGC, rb_intern("disable"), 0);
}


// ----------------------------------------------------------------------------------
//
// convert as_geojson to VALUE
//
VALUE as_geojson_2_val(as_geojson * geo) {
  return rb_funcall(rb_aero_GeoJson, rb_intern("new"), 1, rb_str_new2(geo->value));
}

// ----------------------------------------------------------------------------------
//
// copy as_integer and save it on cheap
//
as_integer * rb_copy_as_integer(as_integer * value) {
  return as_integer_new(value->value);
}

// ----------------------------------------------------------------------------------
//
// copy as_integer and save it on cheap
//
as_integer * rb_copy_as_integer_from_val(as_val * value) {
  return as_integer_new(as_integer_fromval(value)->value);
}

// ----------------------------------------------------------------------------------
//
// copy as_double and save it on cheap
//
as_double * rb_copy_as_double(as_double * value) {
  return as_double_new(value->value);
}

// ----------------------------------------------------------------------------------
//
// copy as_double and save it on cheap
//
as_double * rb_copy_as_double_from_val(as_val * value) {
  return as_double_new(as_double_fromval(value)->value);
}

// ----------------------------------------------------------------------------------
//
// copy as_string and save it on cheap
//
as_string * rb_copy_as_string(as_string * value) {
  char * new_value = (char *) malloc ( sizeof(char *) * ( strlen(value->value) + 1 ) );
  strcpy(new_value, value->value);
  return as_string_new(new_value, true);
}

// ----------------------------------------------------------------------------------
//
// copy as_string and save it on cheap
//
as_string * rb_copy_as_string_from_val(as_val * value) {
  as_string * sval = as_string_fromval(value);
  char * new_value = (char *) malloc ( sizeof(char *) * ( strlen(sval->value) + 1 ) );
  strcpy(new_value, sval->value);
  return as_string_new(new_value, true);
}

// ----------------------------------------------------------------------------------
//
// copy as_geojson and save it on cheap
//
as_geojson * rb_copy_as_geojson(as_geojson * value) {
  char * new_value = (char *) malloc ( sizeof(char *) * ( strlen(value->value) + 1 ) );
  strcpy(new_value, value->value);
  return as_geojson_new(new_value, true);
}

// ----------------------------------------------------------------------------------
//
// copy as_geojson and save it on cheap
//
as_geojson * rb_copy_as_geojson_from_val(as_val * value) {
  as_geojson * sval = as_geojson_fromval(value);
  char * new_value = (char *) malloc ( sizeof(char *) * ( strlen(sval->value) + 1 ) );
  strcpy(new_value, sval->value);
  return as_geojson_new(new_value, true);
}

// ----------------------------------------------------------------------------------
//
// copy as_arraylist and save it on cheap
//
as_arraylist * rb_copy_as_arraylist(as_arraylist * value) {
  as_arraylist_iterator it;
  as_arraylist_iterator_init(&it, value);

  as_arraylist * new_value = as_arraylist_new(value->capacity, 0);

  while ( as_arraylist_iterator_has_next(&it) ) {
    as_val * val = as_arraylist_iterator_next(&it);
    as_arraylist_append(new_value, rb_copy_as_val(val));
  }

  return new_value;
}

// ----------------------------------------------------------------------------------
//
// copy as_arraylist and save it on cheap
//
as_arraylist * rb_copy_as_arraylist_from_val(as_val * value) {
  return rb_copy_as_arraylist(as_list_fromval(value));
}

// ----------------------------------------------------------------------------------
//
// copy as_hashmap and save it on cheap
//
as_hashmap * rb_copy_as_hashmap(as_hashmap * value) {
  as_hashmap_iterator it;
  as_hashmap_iterator_init(&it, value);

  as_hashmap * new_value = as_hashmap_new(value->count);

  while ( as_hashmap_iterator_has_next(&it) ) {
    as_pair * val_pair = as_pair_fromval(as_hashmap_iterator_next(&it));

    as_val * key = val_pair->_1;
    as_val * k_value = val_pair->_2;

    as_hashmap_set(new_value, rb_copy_as_val(key), rb_copy_as_val(k_value));
  }

  return new_value;
}

// ----------------------------------------------------------------------------------
//
// copy as_hashmap and save it on cheap
//
as_hashmap * rb_copy_as_hashmap_from_val(as_val * value) {
  return rb_copy_as_hashmap(as_map_fromval(value));
}

// ----------------------------------------------------------------------------------
//
// copy as_val and save it on cheap
//
as_val * rb_copy_as_val(as_val * value) {
  size_t value_type = as_val_type(value);

  switch ( value_type ) {
    case AS_INTEGER:
      return (as_val *) rb_copy_as_integer_from_val(value);

    case AS_DOUBLE:
      return (as_val *) rb_copy_as_double_from_val(value);

    case AS_STRING:
      return (as_val *) rb_copy_as_string_from_val(value);

    case AS_GEOJSON:
      return (as_val *) rb_copy_as_geojson_from_val(value);

    case AS_LIST:
      return (as_val *) rb_copy_as_arraylist_from_val(value);

    case AS_MAP:
      return (as_val *) rb_copy_as_hashmap_from_val(value);
  }

  return false;
}

// ----------------------------------------------------------------------------------
//
// copy record and save it on cheap
//
as_record * rb_copy_as_record(as_record * record) {
  // init with previous capacity
  as_record * new_record = NULL;
  new_record = as_record_new(record->bins.capacity);
  if (! new_record) rb_raise(rb_aero_MemoryError, "[AerospikeC::Utils] Error while allocating memory for aerospike record");

  as_record_iterator it;
  as_record_iterator_init(&it, record);

  while ( as_record_iterator_has_next(&it) ) {
    as_bin * bin = as_record_iterator_next(&it);
    char *   name = as_bin_get_name(bin);
    as_val * value = (as_val *) as_bin_get_value(bin);

    size_t value_type = as_val_type(value);

    switch ( value_type ) {
      case AS_NIL:
        as_record_set_nil(new_record, name);
        break;

      case AS_INTEGER:
        as_record_set_integer(new_record, name, rb_copy_as_integer_from_val(value));
        break;

      case AS_DOUBLE:
        as_record_set_as_double(new_record, name, rb_copy_as_double_from_val(value));
        break;

      case AS_STRING:
        as_record_set_string(new_record, name, rb_copy_as_string_from_val(value));
        break;

      case AS_GEOJSON:
        as_record_set_geojson(new_record, name, rb_copy_as_geojson_from_val(value));
        break;

      case AS_LIST:
        as_record_set_list(new_record, name, (as_list *)rb_copy_as_arraylist_from_val(value) );
        break;

      case AS_MAP:
        as_record_set_map(new_record, name, (as_map *)rb_copy_as_hashmap_from_val(value));
        break;
    }
  }

  return new_record;
}

// ----------------------------------------------------------------------------------
//
// destroy as_val
//
void as_val_free(as_val * value) {
  size_t value_type = as_val_type(value);

  switch ( value_type ) {
    case AS_INTEGER:
      as_integer_destroy(as_integer_fromval(value));
      return;

    case AS_DOUBLE:
      as_double_destroy(as_double_fromval(value));
      return;

    case AS_STRING:
      as_string_destroy(as_string_fromval(value));
      return;

    case AS_GEOJSON:
      as_geojson_destroy(as_geojson_fromval(value));
      return;

    case AS_LIST:
      as_arraylist_destroy( (as_arraylist *)as_list_fromval(value) );
      return;

    case AS_MAP:
      as_hashmap_destroy( (as_hashmap *)as_map_fromval(value) );
      return;
  }
}

// ----------------------------------------------------------------------------------
//
// convert AerospikeC::Operation to as_operations *
//
as_operations * rb_operations2as_operations(VALUE operations) {
  VALUE rb_ops = rb_iv_get(operations, "@operations");

  int ops_count = rb_ary_len_int(rb_ops);

  as_operations * ops = as_operations_new(ops_count);
  ops->ttl = FIX2INT( rb_iv_get(operations, "@ttl") );

  for (int i = 0; i < ops_count; ++i) {
    VALUE op = rb_ary_entry(rb_ops, i);

    VALUE rb_bin = rb_hash_aref(op, bin_sym);
    char * bin_name;

    if ( rb_bin != Qnil ) {
      bin_name = StringValueCStr(rb_bin);
    }

    VALUE val = rb_hash_aref(op, value_sym);
    VALUE operation_type = rb_hash_aref(op, operation_sym);

    // touch
    if ( operation_type == touch_sym ) {
      as_operations_add_touch(ops);
    }

    // read bin
    else if ( operation_type == read_sym ) {
      as_operations_add_read(ops, bin_name);
    }

    // increment int
    else if ( operation_type == increment_sym ) {
      as_operations_add_incr(ops, bin_name, FIX2LONG(val));
    }

    // append string
    else if ( operation_type == append_sym ) {
      as_operations_add_append_str(ops, bin_name, StringValueCStr(val));
    }

    // prepend string
    else if ( operation_type == prepend_sym ) {
      as_operations_add_prepend_str(ops, bin_name, StringValueCStr(val));
    }

    // write bin
    else if ( operation_type == write_sym ) {
      if ( TYPE(val) == T_FIXNUM ) {
        as_operations_add_write_int64(ops, bin_name, FIX2LONG(val));
      }
      else if ( TYPE(val) == T_STRING ) {
        as_operations_add_write_str(ops, bin_name, StringValueCStr(val));
      }
      else {
        VALUE tmp = value_to_s(val);
        as_operations_add_write_str(ops, bin_name, StringValueCStr(tmp));
      }
    }

    // append list
    else if ( operation_type == list_append_sym ) {
      as_operations_add_list_append(ops, bin_name, rb_val2as_val(val));
    }

    // set at index list
    else if ( operation_type == list_set_sym ) {
      VALUE tmp = rb_hash_aref(op, at_sym);
      as_operations_add_list_set(ops, bin_name, FIX2LONG(tmp), rb_val2as_val(val));
    }

    // trim values not in range <index, count>
    else if ( operation_type == list_trim_sym ) {
      VALUE tmp = rb_hash_aref(op, count_sym);
      as_operations_add_list_trim(ops, bin_name, FIX2LONG(val), FIX2LONG(tmp));
    }

    // clear list
    else if ( operation_type == list_clear_sym ) {
      as_operations_add_list_clear(ops, bin_name);
    }

    // pop from list
    else if ( operation_type == list_pop_sym ) {
      as_operations_add_list_pop(ops, bin_name, FIX2LONG(val));
    }

    // pop_range from list
    else if ( operation_type == list_pop_range_sym ) {
      VALUE tmp = rb_hash_aref(op, count_sym);
      as_operations_add_list_pop_range(ops, bin_name, FIX2LONG(val), FIX2LONG(tmp));
    }

    // pop range from list starting at index
    else if ( operation_type == list_pop_range_from_sym ) {
      as_operations_add_list_pop_range_from(ops, bin_name, FIX2LONG(val));
    }

    // remove from list
    else if ( operation_type == list_remove_sym ) {
      as_operations_add_list_remove(ops, bin_name, FIX2LONG(val));
    }

    // remove_range from list
    else if ( operation_type == list_remove_range_sym ) {
      VALUE tmp = rb_hash_aref(op, count_sym);
      as_operations_add_list_remove_range(ops, bin_name, FIX2LONG(val), FIX2LONG(tmp));
    }

    // remove range from list starting at index
    else if ( operation_type == list_remove_range_from_sym ) {
      as_operations_add_list_remove_range_from(ops, bin_name, FIX2LONG(val));
    }

    // get from list
    else if ( operation_type == list_get_sym ) {
      as_operations_add_list_get(ops, bin_name, FIX2LONG(val));
    }

    // get_range from list
    else if ( operation_type == list_get_range_sym ) {
      VALUE tmp = rb_hash_aref(op, count_sym);
      as_operations_add_list_get_range(ops, bin_name, FIX2LONG(val), FIX2LONG(tmp));
    }

    // get range from list starting at index
    else if ( operation_type == list_get_range_from_sym ) {
      as_operations_add_list_get_range_from(ops, bin_name, FIX2LONG(val));
    }

    // get list size
    else if ( operation_type == list_size_sym ) {
      as_operations_add_list_size(ops, bin_name);
    }

    // uknown operation
    else {
      rb_raise(rb_aero_ParseError, "[AerospikeC::Client][operate] uknown operation type: %s", val_inspect(operation_type));
    }
  }

  return ops;
}