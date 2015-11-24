#include <aerospike_c_ruby.h>

static int foreach_hash2record(VALUE key, VALUE val, VALUE record);
static int foreach_hash2as_hashmap(VALUE key, VALUE val, VALUE map);
static char * key2bin_name(VALUE key);

//
// logger methods
//
void log_debug(const char * msg) {
  if ( TYPE(Logger) != T_OBJECT ) {
    return;
  }
  rb_funcall(Logger, rb_intern("debug"), 1, rb_str_new2(msg));
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

//
// unwrap Client VALUE struct into aerospike *
//
aerospike * get_client_struct(VALUE client) {
  aerospike * as;
  Data_Get_Struct(rb_iv_get(client, "client"), aerospike, as);
  return as;
}

//
// unwrap Key VALUE struct into as_key *
//
as_key * get_key_struct(VALUE key) {
  as_key * k;
  Data_Get_Struct(rb_iv_get(key, "as_key"), as_key, k);
  return k;
}

//
// unwrap Record VALUE struct into as_record *
//
as_record * get_record_struct(VALUE rec) {
  as_record * r;
  Data_Get_Struct(rb_iv_get(rec, "rec"), as_record, r);
  return r;
}

//
// raise RuntimeError with as_error info
//
void raise_as_error(as_error err) {
  log_fatal(err.message);
  rb_raise(rb_eRuntimeError, "%s -- Error code: %d", err.message, err.code);
}

//
// convert as_record to ruby hash
//
VALUE record2hash(as_record * rec) {
  as_arraylist * tmp_list;
  VALUE hash = rb_hash_new();

  as_record_iterator it;
  as_record_iterator_init(&it, rec);

  while ( as_record_iterator_has_next(&it) ) {
    as_bin * bin   = as_record_iterator_next(&it);
    as_val * value = (as_val *) as_bin_get_value(bin);

    VALUE name = rb_str_new2( as_bin_get_name(bin) );

    switch ( as_val_type(value) ) {
      case AS_INTEGER:
        log_debug("[Utils][record2hash] as_val_type(value) -> integer");
        rb_hash_aset(hash, name, as_val_int_2_val(value));
        break;

      case AS_STRING:
        log_debug("[Utils][record2hash] as_val_type(value) -> string");
        rb_hash_aset(hash, name, as_val_str_2_val(value));
        break;

      case AS_LIST:
        log_debug("[Utils][record2hash] as_val_type(value) -> list");
        tmp_list = as_list_fromval(value);
        rb_hash_aset(hash, name, as_list2array(tmp_list));
        break;
    }
  }

  return hash;
}

//
// free memory method
//
static void rec_deallocate(as_record * rec) {
  as_record_destroy(rec);
}

//
// convert ruby hash and save its value to record
//
void hash2record(VALUE hash, VALUE rec) {
  rb_hash_foreach(hash, foreach_hash2record, rec);
}


//
// convert ruby array to as_arraylist
//
as_arraylist array2as_list(VALUE ary) {
  VALUE tmp;
  as_arraylist tmp_list;

  long len = rb_ary_len_long(ary);

  as_arraylist list;
  as_arraylist_init(&list, len, 0);

  for ( long i = 0; i < len; ++i ) {
    VALUE element = rb_ary_entry(ary, i);

    switch ( TYPE(element) ) {
      case T_NIL:
        log_debug("[Utils][array2as_list] TYPE(element) -> nil");
        rb_raise(rb_eRuntimeError, "[array2as_list] Array value cannot be nil");
        break;

      case T_SYMBOL:
        log_debug("[Utils][array2as_list] TYPE(element) -> symbol");
        tmp = rb_funcall(element, rb_intern("to_s"), 0);
        as_arraylist_append_str(&list, StringValueCStr( tmp ));
        break;

      case T_STRING:
        log_debug("[Utils][array2as_list] TYPE(element) -> string");
        as_arraylist_append_str(&list, StringValueCStr( element ));
        break;

      case T_FIXNUM:
        log_debug("[Utils][array2as_list] TYPE(element) -> fixnum");
        as_arraylist_append_int64(&list, FIX2LONG(element));
        break;

      case T_ARRAY:
        log_debug("[Utils][array2as_list] TYPE(element) -> array");
        tmp_list = array2as_list(element);
        as_arraylist_append_list(&list, (as_list *)&tmp_list);
        break;

      default:
        rb_raise(rb_eRuntimeError, "[array2as_list] Unsupported array value type");
        break;
    }
  }

  return list;
}

//
// convert as_list to ruby array
//
VALUE as_list2array(as_arraylist * list) {
  as_arraylist_iterator it;
  as_arraylist_iterator_init(&it, list);

  VALUE ary = rb_ary_new();

  while (as_arraylist_iterator_has_next(&it) ) {
    as_val * value = as_arraylist_iterator_next(&it);

    switch ( as_val_type(value) ) {
      case AS_INTEGER:
        log_debug("[Utils][as_list2array] as_val_type(value) -> integer");
        rb_ary_push(ary, as_val_int_2_val(value));
        break;

      case AS_STRING:
        log_debug("[Utils][as_list2array] as_val_type(value) -> string");
        rb_ary_push(ary, as_val_str_2_val(value));
        break;

      default:
        rb_raise(rb_eRuntimeError, "[as_list2array] Unsupported array value type");
        break;
    }
  }

  return ary;
}

//
// foreach hash2record
//
static int foreach_hash2record(VALUE key, VALUE val, VALUE record) {
  VALUE tmp;
  as_arraylist tmp_list;
  as_hashmap tmp_map;

  char * bin_name = key2bin_name(key);

  as_record * rec;
  Data_Get_Struct(record, as_record, rec);

  switch ( TYPE(val) ) { // set bin_name = val dependent on type
    case T_NIL:
      log_debug("[Utils][foreach_hash2record] TYPE(val) -> nil");
      as_record_set_nil(rec, bin_name);
      break;

    case T_SYMBOL:
      log_debug("[Utils][foreach_hash2record] TYPE(val) -> symbol");
      tmp = rb_funcall(val, rb_intern("to_s"), 0);
      as_record_set_str(rec, bin_name, StringValueCStr(tmp));
      break;

    case T_FIXNUM:
      log_debug("[Utils][foreach_hash2record] TYPE(val) -> fixnum");
      as_record_set_int64(rec, bin_name, FIX2LONG(val));
      break;

    case T_STRING:
      log_debug("[Utils][foreach_hash2record] TYPE(val) -> string");
      as_record_set_str(rec, bin_name, StringValueCStr(val));
      break;

    case T_ARRAY:
      log_debug("[Utils][foreach_hash2record] TYPE(val) -> array");
      tmp_list = array2as_list(val);
      as_record_set_list(rec, bin_name, (as_list *)&tmp_list);
      break;

    // case T_HASH:
    //   tmp_map = hash2as_hashmap(val);
    //   as_record_set_map(rec, bin_name, (as_map *) &tmp_map  );
    //   break;

    default:
      rb_raise(rb_eRuntimeError, "Unsupported record value type");
      break;
  }

  return ST_CONTINUE;
}


//
// convert ruby hash key into char *
//
static char * key2bin_name(VALUE key) {
  VALUE tmp;
  char * bin_name;

  switch ( TYPE(key) ) { // get bin name from key
    case T_NIL:
      log_debug("[Utils][key2bin_name] TYPE(val) nil");
      rb_raise(rb_eRuntimeError, "Record key cannot be nil");
      break;

    case T_SYMBOL:
      log_debug("[Utils][key2bin_name] TYPE(val) symbol");
      tmp = rb_funcall(key, rb_intern("to_s"), 0);
      bin_name = StringValueCStr( tmp );
      break;

    case T_STRING:
      log_debug("[Utils][key2bin_name] TYPE(val) string");
      bin_name = StringValueCStr(key);
      break;

    default:
      rb_raise(rb_eRuntimeError, "Unsupported key type");
      break;
  }

  return bin_name;
}

/**************************************************************************
    @TODO
    problem z mapami, głównie z pamiecia
    segfault praktycznie zawsze
    na pozniej bo nie jest to takie bardzo istotne w projekcie


//
// free memory method
//
void map_deallocate(as_hashmap * map) {
  log_debug("[Utils][map_deallocate] start");
  as_hashmap_destroy(map);
}

//
// convert ruby hash to as_hashmap
//
as_hashmap hash2as_hashmap(VALUE hash) {
  long len = rb_ary_len_long(hash);

  as_hashmap * map = as_hashmap_new(len);

  VALUE hmap = Data_Wrap_Struct(Record, NULL, map_deallocate, map);

  rb_hash_foreach(hash, foreach_hash2as_hashmap, hmap);
}

//
// foreach hash2as_hashmap
//
static int foreach_hash2as_hashmap(VALUE key, VALUE val, VALUE hmap) {
  VALUE tmp;
  as_arraylist tmp_list;
  as_hashmap tmp_map;

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
      as_stringmap_set_list(map, bin_name, tmp_list);
      break;

    default:
      rb_raise(rb_eRuntimeError, "Unsupported record value type");
      break;
  }

  return ST_CONTINUE;

}
**************************************************************************/

//
// ruby array to char **
// remember to destroy allocated memory with inputArray_destroy(inputArray)
//
char ** rb_array2inputArray(VALUE ary) {
  char ** inputArray;

  long len = rb_ary_len_long(ary);

  inputArray = malloc( (len + 1) * sizeof(char *) );

  for ( long i = 0; i < len; ++i ) {
    VALUE element = rb_ary_entry(ary, i);

    char * str = StringValueCStr(element);

    inputArray[i] = malloc( strlen(str) * sizeof(char *) );
    strcpy(inputArray[i], str);
  }

  inputArray[len] = NULL;

  return inputArray;
}

//
// frees inputArray memory
//
void inputArray_destroy(char ** inputArray) {
  for (int i = 0; ; ++i) {
    if ( inputArray[i] == NULL ) {
      free(inputArray[i]);
      break;
    }

    free(inputArray[i]);
  }

  free(inputArray);
}