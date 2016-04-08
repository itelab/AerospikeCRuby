#include <aerospike_c_ruby.h>
#include <client_utils.h>

VALUE rb_aero_Client;
VALUE rb_aero_Logger;

#define aero_KEY_INFO rb_funcall(key, rb_intern("key_info"), 0)

//
// Aerospike queries and scans callback runs in parallel
// Better not touch ruby api inside threads...
//
pthread_mutex_t G_CALLBACK_MUTEX = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

//
// client allocate
//
static VALUE client_allocate(VALUE self) {
  aerospike * as = (aerospike *) ruby_xmalloc ( sizeof(aerospike) );

  return Data_Wrap_Struct(self, NULL, client_deallocate, as);
}

// ----------------------------------------------------------------------------------
//
// def initialize(host, port, options = {})
//
static void client_initialize(int argc, VALUE * argv, VALUE self) {
  struct timeval tm;
  start_timing(&tm);

  VALUE host;
  VALUE port;
  VALUE options;

  rb_scan_args(argc, argv, "21", &host, &port, &options);

  if ( NIL_P(options) ) options = rb_hash_new();

  rb_iv_set(self, "@host", host);
  rb_iv_set(self, "@port", port);
  rb_iv_set(self, "@last_scan_id", Qnil);
  rb_iv_set(self, "@last_query_id", Qnil);

  as_config config;
  as_config_init(&config);
  as_config_add_host(&config, StringValueCStr(host), FIX2INT(port));

  options2config(&config, options, self);
  rb_iv_set(self, "@options", options);

  aerospike * as;
  Data_Get_Struct(self, aerospike, as);

  aerospike_init(as, &config);

  as_error err;
  if (aerospike_connect(as, &err) != AEROSPIKE_OK) {
    aerospike_destroy(as);
    raise_as_error(err);
  }

  log_info_with_time("[Client] initializing and connecting done", &tm);
}

// ----------------------------------------------------------------------------------
//
// adding record to the cluster
//
// def put(key, bins, options = {})
//
// params:
//   key - AerospikeC::Key object
//   bins - either hash {"bin name" => "value"} or AerospikeC::Record object
//   options - hash of options:
//     ttl: time to live record (default: 0)
//     policy: AerospikeC::Policy for write
//
//  ------
//  RETURN:
//    1. true if completed succesfuly
//    2. nil when AEROSPIKE_ERR_RECORD_NOT_FOUND
//
static VALUE put(int argc, VALUE * argv, VALUE self) {
  struct timeval tm;
  start_timing(&tm);

  as_status status;
  as_error err;
  as_record * rec = NULL;
  aerospike * as  = get_client_struct(self);

  VALUE key;
  VALUE hash;
  VALUE options;

  rb_scan_args(argc, argv, "21", &key, &hash, &options);

  // default values for optional arguments
  if ( NIL_P(options) ) {
    options = rb_hash_new();
    rb_hash_aset(options, ttl_sym, rb_zero);
  }
  else {
    VALUE option_tmp = rb_hash_aref(options, ttl_sym);

    if ( NIL_P(option_tmp) )
      rb_hash_aset(options, ttl_sym, rb_zero);

    // if ( TYPE(option_tmp) != T_FIXNUM ) // check ttl option
    //   rb_raise(rb_aero_OptionError, "[AerospikeC::Client][put] ttl must be an integer, options: %s", val_inspect(options));
  }

  VALUE new_rec;

  if ( TYPE(hash) == T_HASH ) {
    new_rec = rb_funcall(rb_aero_Record, rb_intern("new"), 1, hash);
  }
  else {
    new_rec = hash;
  }

  as_key * k = get_key_struct(key);
  as_policy_write * policy = get_policy(options);

  rec = get_record_struct(new_rec);
  rec->ttl = FIX2INT( rb_hash_aref(options, ttl_sym) );

  if ( ( status = aerospike_key_put(as, &err, policy, k, rec) ) != AEROSPIKE_OK) {
    as_record_destroy(rec);

    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[Client][put] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    raise_as_error(err);
  }

  as_record_destroy(rec);

  log_info_with_time_v("[Client][put] success", &tm, aero_KEY_INFO);

  return Qtrue;
}

// ----------------------------------------------------------------------------------
//
// getting record for specifed key
// read all bins of record or given in specific_bins argument
//
// def get(key, specific_bins = nil, options = {})
//
// params:
//   keys - Array of AerospikeC::Key objects
//   specific bins - Array of strings representing bin names
//   options - hash of options:
//     with_header: returns also generation and expire_in field (default: false)
//     policy: AerospikeC::Policy for read
//
//  ------
//  RETURN:
//    1. hash representing record
//    2. nil when AEROSPIKE_ERR_RECORD_NOT_FOUND
//
static VALUE get(int argc, VALUE * argv, VALUE self) {
  struct timeval tm;
  start_timing(&tm);

  as_error err;
  as_status status;
  aerospike * as  = get_client_struct(self);
  as_record * rec = NULL;

  VALUE bins;
  VALUE key;
  VALUE specific_bins;
  VALUE options;

  rb_scan_args(argc, argv, "12", &key, &specific_bins, &options);

  // default values for optional arguments
  if ( NIL_P(specific_bins) ) specific_bins = Qnil;
  if ( NIL_P(options) ) {
    options = rb_hash_new();
    rb_hash_aset(options, with_header_sym, Qfalse);
  }

  as_key * k = get_key_struct(key);
  as_policy_read * policy = get_policy(options);

  // read specific bins
  if ( specific_bins != Qnil && rb_ary_len_int(specific_bins) > 0 ) {
    if ( TYPE(specific_bins) != T_ARRAY ) {
      rb_raise(rb_aero_OptionError, "[AerospikeC::Client][get] specific_bins must be an Array");
    }

    char ** inputArray = rb_array2inputArray(specific_bins); // convert ruby array to char **

    if ( ( status = aerospike_key_select(as, &err, policy, k, inputArray, &rec) ) != AEROSPIKE_OK) {
      as_record_destroy(rec);
      inputArray_destroy(inputArray);

      if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
        log_warn("[Client][get] AEROSPIKE_ERR_RECORD_NOT_FOUND");
        return Qnil;
      }

      raise_as_error(err);
    }

    bins = record2hash(rec);
    bins = check_with_header(bins, options, rec);

    as_record_destroy(rec);
    inputArray_destroy(inputArray);

    check_for_llist_workaround(self, key, bins);

    log_info_with_time_v("[Client][get] success", &tm, aero_KEY_INFO);

    return bins;
  }

  // read all bins
  if ( ( status = aerospike_key_get(as, &err, policy, k, &rec) ) != AEROSPIKE_OK) {
    as_record_destroy(rec);

    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[Client][get] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    raise_as_error(err);
  }

  bins = record2hash(rec);
  bins = check_with_header(bins, options, rec);

  as_record_destroy(rec);

  check_for_llist_workaround(self, key, bins);

  log_info_with_time_v("[Client][get] success", &tm, aero_KEY_INFO);

  return bins;
}

// ----------------------------------------------------------------------------------
//
// removing record from the cluster
//
// def delete(key)
//
// params:
//   key - AerospikeC::Key object
//   options:
//    policy:  AerospikeC::Policy for remove
//
//  ------
//  RETURN:
//    1. true if deleted
//    2. nil if AEROSPIKE_ERR_RECORD_NOT_FOUND
//
static VALUE delete_record(int argc, VALUE * argv, VALUE self) {
  struct timeval tm;
  start_timing(&tm);

  as_error err;
  as_status status;
  aerospike * as = get_client_struct(self);

  VALUE key;
  VALUE options;

  rb_scan_args(argc, argv, "11", &key, &options);

  if ( NIL_P(options) ) options = rb_hash_new();

  as_key * k = get_key_struct(key);
  as_policy_remove * policy = get_policy(options);

  if ( ( status = aerospike_key_remove(as, &err, policy, k) ) != AEROSPIKE_OK ) {
    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[Client][delete] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    raise_as_error(err);
  }

  log_info_with_time_v("[Client][delete] success", &tm, aero_KEY_INFO);

  return Qtrue;
}

// ----------------------------------------------------------------------------------
//
// setting logger object
//
// def logger=(logger)
//
// params:
//   logger - any object that responds to: :debug, :info, :warn, :error, :fatal methods
//
//  ------
//  RETURN:
//    1. true if no errors
//
static VALUE set_logger(VALUE self, VALUE logger) {
  log_info("[Client] Setting logger object");
  rb_aero_Logger = logger;
  return Qtrue;
}

// ----------------------------------------------------------------------------------
//
// check if key exist in cluster
//
// def exists?(key, options = {})
//
// params:
//   key - AerospikeC::Key object
//   options - hash of options:
//     policy: AerospikeC::Policy for read
//
//  ------
//  RETURN:
//    1. true if exist
//    2. false otherwise
//
static VALUE key_exists(int argc, VALUE * argv, VALUE self) {
  struct timeval tm;
  start_timing(&tm);

  as_error err;
  as_status status;
  aerospike * as  = get_client_struct(self);
  as_record * rec = NULL;

  VALUE key;
  VALUE options;

  rb_scan_args(argc, argv, "11", &key, &options);

  if ( NIL_P(options) ) options = rb_hash_new();

  as_key * k = get_key_struct(key);
  as_policy_read * policy = get_policy(options);

  if ( ( status = aerospike_key_exists(as, &err, policy, k, &rec) ) != AEROSPIKE_OK ) {
    as_record_destroy(rec);

    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_info_with_time_v("[Client][exists?] success - false", &tm, aero_KEY_INFO);
      return Qfalse;
    }

    raise_as_error(err);
  }
  else {
    as_record_destroy(rec);
    log_info_with_time_v("[Client][exists?] success - true", &tm, aero_KEY_INFO);
    return Qtrue;
  }
}

// ----------------------------------------------------------------------------------
//
// def get_header(key, options = {})
//
// params:
//   key - AerospikeC::Key object
//   options - hash of options:
//     policy: AerospikeC::Policy for read
//
//  ------
//  RETURN:
//    1. hash representing record header
//    2. nil when AEROSPIKE_ERR_RECORD_NOT_FOUND
//
static VALUE get_header(int argc, VALUE * argv, VALUE self) {
  struct timeval tm;
  start_timing(&tm);

  as_error err;
  as_status status;
  aerospike * as  = get_client_struct(self);
  as_record * rec = NULL;

  VALUE key;
  VALUE options;

  rb_scan_args(argc, argv, "11", &key, &options);

  if ( NIL_P(options) ) options = rb_hash_new();

  as_key * k = get_key_struct(key);
  as_policy_read * policy = get_policy(options);

  VALUE header = rb_hash_new();

  const char * inputArray[] = { NULL };

  if ( ( status = aerospike_key_select(as, &err, policy, k, inputArray, &rec) ) != AEROSPIKE_OK) {
    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[Client][get_header] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    raise_as_error(err);
  }

  rb_hash_aset(header, rb_str_new2("gen"), INT2FIX(rec->gen));
  rb_hash_aset(header, rb_str_new2("expire_in"), INT2FIX(rec->ttl));

  as_record_destroy(rec);

  log_info_with_time_v("[Client][get_header] success", &tm, aero_KEY_INFO);

  return header;
}

// ----------------------------------------------------------------------------------
//
// getting batch of records in one call
// batch size is limited on aerospike server (default: 5000)
//
// def batch_get(keys, specific_bins = nil, options = {})
//
// params:
//   keys - Array of AerospikeC::Key objects
//   specific bins - Array of strings representing bin names
//   options - hash of options:
//     with_header: returns also generation and expire_in field (default: false)
//
//  ------
//  RETURN: Array of hashes where each hash represents record bins
//
// @TODO options policy
//
static VALUE batch_get(int argc, VALUE * argv, VALUE self) {
  struct timeval tm;
  start_timing(&tm);

  as_error err;
  as_status status;
  aerospike * as = get_client_struct(self);
  char ** bin_names;
  long n_bin_names;

  VALUE keys;
  VALUE specific_bins;
  VALUE options;

  rb_scan_args(argc, argv, "12", &keys, &specific_bins, &options);

  // default values for optional arguments
  if ( NIL_P(specific_bins) ) {
    specific_bins = Qnil;
  }
  else {
    if ( TYPE(specific_bins) != T_ARRAY ) rb_raise(rb_aero_OptionError, "[AerospikeC::Client][batch_get] specific_bins must be an Array");

    bin_names   = rb_array2bin_names(specific_bins);
    n_bin_names = rb_ary_len_long(specific_bins);
  }
  if ( NIL_P(options) ) {
    options = rb_hash_new();
    rb_hash_aset(options, with_header_sym, Qfalse);
  }

  long keys_len = rb_ary_len_long(keys);

  VALUE records_bins = rb_ary_new();

  as_batch_read_records records;
  as_batch_read_inita(&records, keys_len);

  // map array into as_batch_read_record * record
  for (int i = 0; i < keys_len; ++i) {
    VALUE element = rb_ary_entry(keys, i);
    VALUE tmp;

    tmp = rb_funcall(element, rb_intern("namespace"), 0);
    char * c_namespace = StringValueCStr( tmp );

    tmp = rb_funcall(element, rb_intern("set"), 0);
    char * c_set = StringValueCStr( tmp );

    as_batch_read_record * record = as_batch_read_reserve(&records);

    tmp = rb_funcall(element, rb_intern("key"), 0);

    if ( TYPE(tmp) != T_FIXNUM ) {
      char * c_key = StringValueCStr( tmp );
      as_key_init(&record->key, c_namespace, c_set, c_key);
    }
    else {
      as_key_init_int64(&record->key, c_namespace, c_set, FIX2LONG(tmp));
    }

    if ( specific_bins == Qnil ) {
      record->read_all_bins = true;
    }
    else {
      record->bin_names  = bin_names;
      record->n_bin_names = n_bin_names;
    }
  }

  // read here!
  if ( ( status = aerospike_batch_read(as, &err, NULL, &records) ) != AEROSPIKE_OK ) {
    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[Client][batch_get] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    as_batch_read_destroy(&records);
    raise_as_error(err);
  }

  as_vector list = records.list;

  // map records into array of hashes
  for (long i = 0; i < list.size; ++i) {
    as_batch_read_record * record = as_vector_get(&list, i);
    as_record rec = record->record;

    VALUE bins = record2hash(&rec);
    bins = check_with_header(bins, options, &rec);

    rb_ary_push(records_bins, bins);
  }

  as_batch_read_destroy(&records);

  if ( specific_bins != Qnil ) bin_names_destroy(bin_names, n_bin_names);

  log_info_with_time("[Client][batch_get] success", &tm);

  return records_bins;
}

// ----------------------------------------------------------------------------------
//
// def touch(key, options = {})
//
// params:
//   key - AeropsikeC::Key object
//   options - hash of options:
//     ttl: time to live record (default: 0)
//     policy: AerospikeC::Policy for operate
//
//  ------
//  RETURN:
//    1. hash representing record header
//    2. nil when AEROSPIKE_ERR_RECORD_NOT_FOUND
//
// @TODO options policy
//
static VALUE touch(int argc, VALUE * argv, VALUE self) {
  struct timeval tm;
  start_timing(&tm);

  as_error err;
  as_status status;
  aerospike * as = get_client_struct(self);

  VALUE key;
  VALUE options;

  rb_scan_args(argc, argv, "11", &key, &options);

  // default values for optional arguments
  if ( NIL_P(options) ) {
    options = rb_hash_new();
    rb_hash_aset(options, ttl_sym, rb_zero);
  }
  else {
    if ( TYPE(rb_hash_aref(options, ttl_sym)) != T_FIXNUM ) { // check ttl option
      rb_raise(rb_aero_OptionError, "[AerospikeC::Client][put] ttl must be an integer");
    }
  }

  as_key * k = get_key_struct(key);
  as_record * rec = NULL;

  as_operations ops;
  as_operations_inita(&ops, 1);
  as_operations_add_touch(&ops);

  ops.ttl = FIX2INT( rb_hash_aref(options, ttl_sym) );

  as_policy_operate * policy = get_policy(options);

  if ( ( status = aerospike_key_operate(as, &err, policy, k, &ops, &rec) ) != AEROSPIKE_OK ) {
    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[Client][touch] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    as_operations_destroy(&ops);
    raise_as_error(err);
  }

  VALUE header = rb_hash_new();

  rb_hash_aset(header, rb_str_new2("gen"), INT2FIX(rec->gen));
  rb_hash_aset(header, rb_str_new2("expire_in"), INT2FIX(rec->ttl));

  as_record_destroy(rec);
  as_operations_destroy(&ops);

  log_info_with_time_v("[Client][touch] success", &tm, aero_KEY_INFO);

  return header;
}

// ----------------------------------------------------------------------------------
//
// perform given operations on record in one call
//
// def operate(key, operations, options = {})
//
// params:
//   key - AeropsikeC::Key object
//   operations - AeropsikeC::Operation object
//   options:
//     policy: AerospikeC::Policy for operate
//
//  ------
//  RETURN:
//    1. hash representing record
//    2. nil when AEROSPIKE_ERR_RECORD_NOT_FOUND
//
//
static VALUE operate(int argc, VALUE * argv, VALUE self) {
  struct timeval tm;
  start_timing(&tm);

  as_error err;
  as_status status;
  aerospike * as = get_client_struct(self);

  VALUE key;
  VALUE operations;
  VALUE options;

  rb_scan_args(argc, argv, "21", &key, &operations, &options);

  if ( NIL_P(options) ) {
    options = rb_hash_new();
  }

  as_key * k     = get_key_struct(key);

  VALUE is_aerospike_c_operation = rb_funcall(operations, rb_intern("is_a?"), 1, rb_const_get(rb_aero_AerospikeC, rb_intern("Operation")));
  if ( is_aerospike_c_operation != Qtrue ) {
    rb_raise(rb_aero_OptionError, "[AerospikeC::Client][operate] use AerospikeC::Operation class to perform operations");
  }

  as_operations * ops = rb_operations2as_operations(operations);

  as_record * rec = NULL;
  as_policy_operate * policy = get_policy(options);

  if ( ( status = aerospike_key_operate(as, &err, policy, k, ops, &rec) ) != AEROSPIKE_OK ) {
    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[Client][operate] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    as_operations_destroy(&ops);
    raise_as_error(err);
  }

  VALUE record = record2hash(rec);

  as_record_destroy(rec);
  as_operations_destroy(ops);

  log_info_with_time_v("[Client][operate] success", &tm, aero_KEY_INFO);

  return record;
}

// ----------------------------------------------------------------------------------
//
// def operation
//
//  ------
//  RETURN:
//    1. new AerospikeC::Operation object
//
static VALUE operation_obj(VALUE self) {
  log_info("[Client] Getting new Operation object");
  return rb_funcall(rb_const_get(rb_aero_AerospikeC, rb_intern("Operation")), rb_intern("new"), 0);
}

// ----------------------------------------------------------------------------------
//
// def create_index(namespace, set, bin, name, data_type, options = {})
//
// params:
//   namespace - string, namespace to be indexed
//   set - string, set to be indexed
//   bin - string, bin or complex position name to be indexed
//   name - string, name of the index
//   data_type = symbol, data type of index, :string or :numeric
//
//  ------
//  RETURN:
//    1. AerospikeC::IndexTask object
//
// @TODO options policy
//
static VALUE create_index(int argc, VALUE * argv, VALUE self) {
  struct timeval tm;
  start_timing(&tm);

  as_error err;
  aerospike * as = get_client_struct(self);

  VALUE ns;
  VALUE set;
  VALUE bin;
  VALUE name;
  VALUE data_type;
  VALUE options;

  rb_scan_args(argc, argv, "51", &ns, &set, &bin, &name, &data_type, &options);

  // default values for optional arguments
  if ( NIL_P(options) ) options = rb_hash_new();

  int d_type;

  if ( data_type == numeric_sym ) {
    d_type = AS_INDEX_NUMERIC;
  }
  else if ( data_type == string_sym ) {
    d_type = AS_INDEX_STRING;
  }
  else if  ( data_type == geo_sphere_sym ) {
    d_type = AS_INDEX_GEO2DSPHERE;
  }
  else {
    rb_raise(rb_aero_OptionError, "[AerospikeC::Client][create_index] data_type must be :string or :numeric");
  }

  as_index_task * task = (as_index_task *) ruby_xmalloc( sizeof(as_index_task) );
  if (! task) rb_raise(rb_aero_MemoryError, "[AerospikeC::Client][create_index] Error while allocating memory for aerospike task");


  if ( aerospike_index_create(as, &err, task, NULL, StringValueCStr(ns), StringValueCStr(set),
                              StringValueCStr(bin), StringValueCStr(name), d_type) != AEROSPIKE_OK )
    raise_as_error(err);

  VALUE index_task_struct = Data_Wrap_Struct(rb_aero_IndexTask, NULL, index_task_deallocate, task);

  log_info_with_time("[Client][create_index] done", &tm);

  return rb_funcall(rb_aero_IndexTask, rb_intern("new"), 1, index_task_struct);
}

// ----------------------------------------------------------------------------------
//
// def drop_index(namespace, name, options = {})
//
// params:
//   namespace - string, namespace to be indexed
//   name - string, name of the index
//
//  ------
//  RETURN:
//    1. true if drop executed correctly
//
// @TODO options policy
//
static VALUE drop_index(int argc, VALUE * argv, VALUE self) {
  struct timeval tm;
  start_timing(&tm);

  as_error err;
  aerospike * as = get_client_struct(self);

  VALUE ns;
  VALUE name;
  VALUE options;

  rb_scan_args(argc, argv, "21", &ns, &name, &options);

  // default values for optional arguments
  if ( NIL_P(options) ) options = rb_hash_new();

  if ( aerospike_index_remove(as, &err, NULL, StringValueCStr(ns), StringValueCStr(name)) != AEROSPIKE_OK )
    raise_as_error(err);

  log_info_with_time("[Client][drop_index] done", &tm);

  return Qtrue;
}

// ----------------------------------------------------------------------------------
//
// execute info command
// http://www.aerospike.com/docs/reference/info/
//
// def info_cmd(cmd)
//
// params:
//   cmd - string, info command to execute
//
//  ------
//  RETURN:
//    1. msg from server
//
// @TODO options policy
//
static VALUE info_cmd(VALUE self, VALUE cmd) {
  struct timeval tm;
  start_timing(&tm);

  as_error err;
  aerospike * as = get_client_struct(self);

  char * res = NULL;

  VALUE host = rb_iv_get(self, "@host");
  VALUE port = rb_iv_get(self, "@port");

  if ( aerospike_info_host(as, &err, NULL, StringValueCStr(host), FIX2INT(port), StringValueCStr(cmd), &res) != AEROSPIKE_OK ) {
    raise_as_error(err);
  }

  VALUE info_res = rb_str_new2(res);
  free(res);

  log_info_with_time("[Client][info_cmd] done", &tm);

  return info_res;
}

// ----------------------------------------------------------------------------------
//
// register udf from file
//
// def register_udf(path_to_file, server_path, language = :lua, options = {})
//
// params:
//   path_to_file - absolute path to udf file
//   server_path - where to put udf on the server
//   language - udf language (in aerospike-c-client v3.1.24, only lua language is available)
//
//  ------
//  RETURN:
//    1. AerospikeC::UdfTask object
//
// @TODO options policy
//
static VALUE register_udf(int argc, VALUE * argv, VALUE self) {
  struct timeval tm;
  start_timing(&tm);

  as_error err;
  aerospike * as = get_client_struct(self);

  VALUE path_to_file;
  VALUE server_path;
  VALUE language;
  VALUE options;

  rb_scan_args(argc, argv, "22", &path_to_file, &server_path, &language, &options);

  // default values for optional arguments
  if ( NIL_P(options) ) options = rb_hash_new();

  if ( NIL_P(language) ) {
    language = lua_sym;
  }
  else {
    if ( language != lua_sym ) rb_raise(rb_aero_OptionError, "[AerospikeC::Client][register_udf] in aerospike-c-client v3.1.24, only lua language is available");
  }

  FILE* file = fopen(StringValueCStr(path_to_file), "r");

  if (! file) rb_raise(rb_aero_OptionError, "[AerospikeC::Client][register_udf] Cannot read udf from given path: %s", StringValueCStr(path_to_file));

  uint8_t * content = (uint8_t *) malloc(1024 * 1024);
  if (! content) rb_raise(rb_aero_MemoryError, "[AerospikeC::Client][register_udf] Error while allocating memory for udf file content");

  // read the file content into a local buffer
  uint8_t * p_write = content;
  int read = (int)fread(p_write, 1, 512, file);
  int size = 0;

  while (read) {
    size += read;
    p_write += read;
    read = (int)fread(p_write, 1, 512, file);
  }

  fclose(file);

  as_bytes udf_content;
  as_bytes_init_wrap(&udf_content, content, size, true);

  // register the UDF file in the database cluster
  if ( aerospike_udf_put(as, &err, NULL, StringValueCStr(server_path), AS_UDF_TYPE_LUA, &udf_content) != AEROSPIKE_OK ) {
    as_bytes_destroy(&udf_content);
    raise_as_error(err);
  }

  as_bytes_destroy(&udf_content);

  log_info_with_time("[Client][register_udf] success", &tm);

  return rb_funcall(rb_aero_UdfTask, rb_intern("new"), 2, server_path, self);
}

// ----------------------------------------------------------------------------------
//
// drop udf for given server_path
//
// def drop_udf(server_path, options = {})
//
// params:
//   server_path - where is udf on the server (name)
//
//  ------
//  RETURN:
//    1. true if succesfull
//
// @TODO options policy
//
static VALUE drop_udf(int argc, VALUE * argv, VALUE self) {
  struct timeval tm;
  start_timing(&tm);

  as_error err;
  aerospike * as = get_client_struct(self);

  VALUE server_path;
  VALUE options;

  rb_scan_args(argc, argv, "11", &server_path, &options);

  // default values for optional arguments
  if ( NIL_P(options) ) options = rb_hash_new();

  if ( aerospike_udf_remove(as, &err, NULL, StringValueCStr(server_path)) != AEROSPIKE_OK )
    raise_as_error(err);

  log_info_with_time("[Client][drop_udf] success", &tm);

  return Qtrue;
}

// ----------------------------------------------------------------------------------
//
// list all udfs
//
// def list_udf(options = {})
//
// params:
//   options - ?
//
//  ------
//  RETURN:
//    1. array of hashes representing each udf
//
// @TODO options policy
//
static VALUE list_udf(int argc, VALUE * argv, VALUE self) {
  as_error err;
  aerospike * as = get_client_struct(self);

  VALUE options;

  rb_scan_args(argc, argv, "01", &options);

  // default values for optional arguments
  if ( NIL_P(options) ) options = rb_hash_new();

  as_udf_files files;
  as_udf_files_init(&files, 0);

  if ( aerospike_udf_list(as, &err, NULL, &files) != AEROSPIKE_OK )
    raise_as_error(err);

  VALUE udfs = rb_ary_new();

  for( int i = 0; i < files.size; i++ ) {
    as_udf_file * file = &files.entries[i];

    VALUE udf_file = rb_hash_new();
    VALUE udf_type;

    if ( file->type == 0 ) udf_type = lua_sym;

    rb_hash_aset(udf_file, name_sym, rb_str_new2(file->name));
    rb_hash_aset(udf_file, udf_type_sym, udf_type);
    rb_hash_aset(udf_file, hash_sym, INT2FIX(file->hash));

    rb_ary_push(udfs, udf_file);
  }

  as_udf_files_destroy(&files);

  return udfs;
}

// ----------------------------------------------------------------------------------
//
// execute udf on record
//
// def execute_udf(key, module_name, func_name, udf_args = [], options = {})
//
// params:
//   key - AeropsikeC::Key object
//   module_name - string, registered module name
//   func_name - string, function name in module to execute
//   udf_args - arguments passed to udf
//   options:
//     policy - AerospikeC::ApplyPolicy
//
//  ------
//  RETURN:
//    1. data returned from udf
//
static VALUE execute_udf(int argc, VALUE * argv, VALUE self) {
  struct timeval tm;
  start_timing(&tm);

  as_error err;
  as_status status;
  aerospike * as = get_client_struct(self);

  VALUE key;
  VALUE module_name;
  VALUE func_name;
  VALUE udf_args;
  VALUE options;

  rb_scan_args(argc, argv, "32", &key, &module_name, &func_name, &udf_args, &options);

  // default values for optional arguments
  if ( NIL_P(options) ) options = rb_hash_new();
  if ( NIL_P(udf_args) ) udf_args = rb_ary_new();

  as_key * k               = get_key_struct(key);
  as_arraylist * args      = array2as_list(udf_args);
  as_policy_apply * policy = get_policy(options);

  as_val * res = NULL;

  char * c_module_name = StringValueCStr(module_name);
  char * c_func_name   = StringValueCStr(func_name);

  if ( ( status = aerospike_key_apply(as, &err, policy, k, c_module_name, c_func_name, (as_list *)args, &res) ) != AEROSPIKE_OK ) {
    as_arraylist_destroy(args);

    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[Client][execute_udf] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    raise_as_error(err);
  }

  VALUE result = as_val2rb_val(res);

  as_val_destroy(&res);
  as_arraylist_destroy(args);

  VALUE key_info = aero_KEY_INFO;
  VALUE mod_info = rb_sprintf("mod: %s, func: %s", c_module_name, c_func_name);

  log_info_with_time_v2("[Client][execute_udf] success", &tm, key_info, mod_info);

  return result;
}

// ----------------------------------------------------------------------------------
//
// callback for scan_records
// push into array only if non error
//
static VALUE scan_records_callback_protected(VALUE rdata) {
  as_val * val = (as_val *) rdata;

  as_record * record = as_rec_fromval(val);

  return record2hash(record);
}

static bool scan_records_callback(const as_val * val, VALUE scan_data) {
  if ( val == NULL ) return true;

  pthread_mutex_lock(& G_CALLBACK_MUTEX); // lock

  int state = 0;
  VALUE result = rb_protect(scan_records_callback_protected, (VALUE)(val), &state);

  if (!state)
    rb_ary_push(scan_data, result);

  pthread_mutex_unlock(& G_CALLBACK_MUTEX); // unlock

  return true;
}

// ----------------------------------------------------------------------------------
//
// scan records in specified namespace and set
// multiple threads will likely be calling the callback in parallel so return data won't be sorted
//
// def scan(ns, set, options = {})
//
// params:
//   ns - namespace to scan
//   set - set to scan
//   options:
//     - priority - scan priority
//
//  ------
//  RETURN:
//    1. data returned from scan
//
// @TODO options policy
//
static VALUE scan_records_begin(VALUE rdata) {
  scan_method_options * args = (scan_method_options *) rdata;
  as_error err;

  disable_rb_GC();

  if ( aerospike_scan_foreach(args->as, &err, args->policy, args->scan, args->callback, args->scan_data) != AEROSPIKE_OK )
    raise_as_error(err);

  return args->scan_data;
}

static VALUE scan_records_ensure(VALUE rdata) {
  scan_method_options * args = (scan_method_options *) rdata;

  enable_rb_GC();
  as_scan_destroy(args->scan);

  return args->scan_data;
}

static VALUE scan_records(int argc, VALUE * argv, VALUE self) {
  struct timeval tm;
  start_timing(&tm);

  aerospike * as = get_client_struct(self);

  VALUE ns;
  VALUE set;
  VALUE options;

  rb_scan_args(argc, argv, "21", &ns, &set, &options);

  // default values for optional arguments
  if ( NIL_P(options) ) {
    options = rb_hash_new();
  }

  as_scan scan;
  as_scan_init(&scan, StringValueCStr(ns), StringValueCStr(set));
  set_priority_options(&scan, options);

  VALUE scan_data = rb_ary_new();

  scan_method_options s_args;

  s_args.as        = as;
  s_args.scan      = &scan;
  s_args.policy    = NULL;
  s_args.callback  = scan_records_callback;
  s_args.scan_data = scan_data;

  VALUE result = rb_ensure(scan_records_begin, (VALUE)(&s_args), scan_records_ensure, (VALUE)(&s_args));;

  log_info_with_time("[Client][scan] success", &tm);

  return result;
}

// ----------------------------------------------------------------------------------
//
// execute udf on scan records in specified namespace and set
// multiple threads will likely be calling the callback in parallel so return data won't be sorted
//
// def execute_udf_on_scan(ns, set, module_name, func_name, udf_args = [], options = {})
//
// params:
//   ns - namespace to scan
//   set - set to scan
//   module_name - string, registered module name
//   func_name - string, function name in module to execute
//   udf_args - arguments passed to udf
//   options:
//     - priority - scan priority
//
//  ------
//  RETURN:
//    1. data returned from scan
//
// @TODO options policy
//
static VALUE execute_udf_on_scan_begin(VALUE rdata) {
  scan_method_options * args = (scan_method_options *) rdata;
  as_error err;

  disable_rb_GC();

  if ( aerospike_scan_foreach(args->as, &err, args->policy, args->scan, args->callback, args->scan_data) != AEROSPIKE_OK )
    raise_as_error(err);

  return args->scan_data;
}

static VALUE execute_udf_on_scan_ensure(VALUE rdata) {
  scan_method_options * args = (scan_method_options *) rdata;

  enable_rb_GC();

  as_scan_destroy(args->scan);
  as_arraylist_destroy(args->args);

  return args->scan_data;
}

static VALUE execute_udf_on_scan(int argc, VALUE * argv, VALUE self) {
  struct timeval tm;
  start_timing(&tm);

  aerospike * as = get_client_struct(self);

  VALUE ns;
  VALUE set;
  VALUE module_name;
  VALUE func_name;
  VALUE udf_args;
  VALUE options;

  rb_scan_args(argc, argv, "42", &ns, &set, &module_name, &func_name, &udf_args, &options);

  // default values for optional arguments
  if ( NIL_P(options) ) options = rb_hash_new();

  as_arraylist * args = NULL;
  if ( !(NIL_P(udf_args)) ) {
    args = array2as_list(udf_args);
  }


  as_scan scan;
  as_scan_init(&scan, StringValueCStr(ns), StringValueCStr(set));

  char * c_module_name = StringValueCStr(module_name);
  char * c_func_name = StringValueCStr(func_name);

  as_scan_apply_each(&scan, c_module_name, c_func_name, (as_list *)args);
  set_priority_options(&scan, options);

  VALUE scan_data = rb_ary_new();

  scan_method_options s_args;

  s_args.as        = as;
  s_args.scan      = &scan;
  s_args.policy    = NULL;
  s_args.callback  = scan_records_callback;
  s_args.args      = args;
  s_args.scan_data = scan_data;

  VALUE result = rb_ensure(scan_records_begin, (VALUE)(&s_args), scan_records_ensure, (VALUE)(&s_args));;

  VALUE mod_info = rb_sprintf("mod: %s, func: %s", c_module_name, c_func_name);

  log_info_with_time_v("[Client][scan_udf] success", &tm, mod_info);

  return result;
}

// ----------------------------------------------------------------------------------
//
// execute udf on scan records in specified namespace and set in background
//
// def background_execute_udf_on_scan(ns, set, module_name, func_name, udf_args = [], options = {})
//
// params:
//   ns - namespace to scan
//   set - set to scan
//   module_name - string, registered module name
//   func_name - string, function name in module to execute
//   udf_args - arguments passed to udf
//   options:
//     - priority: scan priority
//     - policy: AerospikeC::Policy for write
//
//  ------
//  RETURN:
//    1. AerospikeC::ScanTask object
//
static VALUE background_execute_udf_on_scan(int argc, VALUE * argv, VALUE self) {
  struct timeval tm;
  start_timing(&tm);

  as_error err;
  aerospike * as = get_client_struct(self);

  VALUE ns;
  VALUE set;
  VALUE module_name;
  VALUE func_name;
  VALUE udf_args;
  VALUE options;

  rb_scan_args(argc, argv, "42", &ns, &set, &module_name, &func_name, &udf_args, &options);

  // default values for optional arguments
  if ( NIL_P(options) ) options = rb_hash_new();

  as_arraylist * args = NULL;
  if ( !(NIL_P(udf_args)) ) {
    args = array2as_list(udf_args);
  }

  as_policy_write * policy = get_policy(options);

  as_scan scan;
  as_scan_init(&scan, StringValueCStr(ns), StringValueCStr(set));

  char * c_module_name = StringValueCStr(module_name);
  char * c_func_name = StringValueCStr(func_name);

  as_scan_apply_each(&scan, c_module_name, c_func_name, (as_list *)args);
  set_priority_options(&scan, options);

  uint64_t scanid = 0;

  if ( aerospike_scan_background(as, &err, policy, &scan, &scanid) != AEROSPIKE_OK )
    raise_as_error(err);

  VALUE scan_id = ULONG2NUM(scanid);

  rb_iv_set(self, "@last_scan_id", scan_id);

  as_scan_destroy(&scan);
  as_arraylist_destroy(args);

  VALUE mod_info = rb_sprintf("mod: %s, func: %s", c_module_name, c_func_name);

  log_info_with_time_v("[Client][bg_scan_udf] success", &tm, mod_info);

  return rb_funcall(rb_aero_ScanTask, rb_intern("new"), 2, scan_id, self);
}

// ----------------------------------------------------------------------------------
//
// callback method for execute_query
// push into array only if non error
//
// static VALUE execute_query_callback_protected(VALUE rdata) {
//   as_val * val = (as_val *) rdata;

//   as_record * record = as_rec_fromval(val);

//   return record2hash(record);
// }

static bool execute_query_callback(as_val * val, query_item * query_data) {
  if ( val == NULL ) return false;

  query_item * new_item = (query_item *) malloc ( sizeof(query_item) );
  if (! new_item) {
    pthread_mutex_lock(& G_CALLBACK_MUTEX); // lock
      log_warn("execute_query_callback allocating memory for query item failed, continue with next record");
    pthread_mutex_unlock(& G_CALLBACK_MUTEX); // unlock
    return true;
  }

  init_query_item(new_item);

  new_item->rec = rb_copy_as_record( as_rec_fromval(val) );

  if (! new_item->rec) {
    pthread_mutex_lock(& G_CALLBACK_MUTEX); // lock
      log_warn("execute_query_callback allocating memory for record failed, continue with next record");
    pthread_mutex_unlock(& G_CALLBACK_MUTEX); // unlock

    return true;
  }

  set_query_item_next(query_data, new_item);

  return true;
}

// ----------------------------------------------------------------------------------
//
// execute query
// multiple threads will likely be calling the callback in parallel so return data won't be sorted
//
// def query(query_obj)
//
// params:
//   query_obj - AeropsikeC::Query object
//
//  ------
//  RETURN:
//    1. data returned from query
//
static VALUE execute_query_begin(VALUE rdata) {
  query_list * args = (query_list *) rdata;
  as_error err;

  if ( aerospike_query_foreach(args->as, &err, args->policy, args->query, args->callback, args->query_data) != AEROSPIKE_OK )
    raise_as_error(err);

  set_query_result_and_destroy(args);

  return args->result;
}

static VALUE execute_query_ensure(VALUE rdata) {
  query_list * args = (query_list *) rdata;

  as_query_destroy(args->query);
  query_result_destroy(args);

  return args->result;
}

static VALUE execute_query(VALUE self, VALUE query_obj) {
  struct timeval tm;
  start_timing(&tm);

  aerospike * as = get_client_struct(self);

  VALUE is_aerospike_c_query_obj = rb_funcall(query_obj, rb_intern("is_a?"), 1, rb_aero_Query);
  if ( is_aerospike_c_query_obj != Qtrue )
    rb_raise(rb_aero_OptionError, "[AerospikeC::Client][query] use AerospikeC::Query class to perform queries");

  as_query * query         = query_obj2as_query(query_obj);
  as_policy_query * policy = get_query_policy(query_obj);

  query_item * query_data = (query_item *) malloc ( sizeof(query_item) );
  if (! query_data) rb_raise(rb_aero_MemoryError, "[AerospikeC::Client][query] Error while allocating memory for query result");
  init_query_item(query_data);

  query_list q_args;

  q_args.as         = as;
  q_args.policy     = policy;
  q_args.query      = query;
  q_args.query_data = query_data;
  q_args.callback   = execute_query_callback;
  q_args.result     = rb_ary_new();

  VALUE result = rb_ensure(execute_query_begin, (VALUE)(&q_args), execute_query_ensure, (VALUE)(&q_args));

  log_info_with_time("[Client][query] success", &tm);

  return result;
}

// ----------------------------------------------------------------------------------
//
// callback method for execute_udf_on_query
// push into array only if non error
// //
// static VALUE execute_udf_on_query_callback_protected(VALUE rdata) {
//   as_val * val = (as_val *) rdata;

//   VALUE tmp;
//   as_record * record;

//   switch ( as_val_type(val) ) {
//     case AS_REC:
//       record = as_rec_fromval(val);
//       tmp = record2hash(record);
//       break;

//     default:
//       tmp = as_val2rb_val(val);
//       break;
//   }

//   return tmp;
// }

static bool execute_udf_on_query_callback(as_val * val, VALUE query_data) {
  if ( val == NULL ) return false;

  query_item * new_item = (query_item *) malloc ( sizeof(query_item) );
  if (! new_item) {
    pthread_mutex_lock(& G_CALLBACK_MUTEX); // lock
      log_warn("execute_udf_on_query_callback allocating memory for query item failed, continue with next record");
    pthread_mutex_unlock(& G_CALLBACK_MUTEX); // unlock
    return true;
  }

  init_query_item(new_item);

  size_t value_type = as_val_type(val);

  if ( value_type == AS_REC ) { // is record
    new_item->rec = rb_copy_as_record( as_rec_fromval(val) );

    if (! new_item->rec) {
      pthread_mutex_lock(& G_CALLBACK_MUTEX); // lock
        log_warn("execute_udf_on_query_callback allocating memory for record failed, continue with next record");
      pthread_mutex_unlock(& G_CALLBACK_MUTEX); // unlock

      return true;
    }
  }
  else { // is value
    as_val_reserve(val);
    new_item->val = val;

    if (! new_item->val) {
      pthread_mutex_lock(& G_CALLBACK_MUTEX); // lock
        log_warn("execute_udf_on_query_callback allocating memory for value failed, continue with next record");
      pthread_mutex_unlock(& G_CALLBACK_MUTEX); // unlock

      return true;
    }
  }

  set_query_item_next(query_data, new_item);

  return true;
}

// ----------------------------------------------------------------------------------
//
// execute udf on query
// multiple threads will likely be calling the callback in parallel so return data won't be sorted
//
// def execute_udf_on_query(query_obj, module_name, func_name, udf_args = [])
//
// params:
//   query_obj - AeropsikeC::Query object
//   module_name - string, registered module name
//   func_name - string, function name in module to execute
//   udf_args - arguments passed to udf
//
//  ------
//  RETURN:
//    1. data returned from query
//
static VALUE execute_udf_on_query_begin(VALUE rdata) {
  query_list * args = (query_list *) rdata;
  as_error err;

  if ( aerospike_query_foreach(args->as, &err, args->policy, args->query, args->callback, args->query_data) != AEROSPIKE_OK )
    raise_as_error(err);

  set_query_result_and_destroy(args);

  return args->result;
}

static VALUE execute_udf_on_query_ensure(VALUE rdata) {
  query_list * args = (query_list *) rdata;

  as_query_destroy(args->query);
  // as_arraylist_destroy(args->args);
  query_result_destroy(args);

  return args->result;
}

static VALUE execute_udf_on_query(int argc, VALUE * argv, VALUE self)  {
  struct timeval tm;
  start_timing(&tm);

  aerospike * as = get_client_struct(self);

  VALUE query_obj;
  VALUE module_name;
  VALUE func_name;
  VALUE udf_args;

  rb_scan_args(argc, argv, "31", &query_obj, &module_name, &func_name, &udf_args);

  VALUE is_aerospike_c_query_obj = rb_funcall(query_obj, rb_intern("is_a?"), 1, rb_aero_Query);
  if ( is_aerospike_c_query_obj != Qtrue )
    rb_raise(rb_aero_OptionError, "[AerospikeC::Client][aggregate] use AerospikeC::Query class to perform queries");

  as_arraylist * args = NULL;

  if ( !(NIL_P(udf_args)) ) {
    args = array2as_list(udf_args);
  }

  as_query * query         = query_obj2as_query(query_obj);
  as_policy_query * policy = get_query_policy(query_obj);

  char * c_module_name = StringValueCStr(module_name);
  char * c_func_name = StringValueCStr(func_name);

  as_query_apply(query, c_module_name, c_func_name, (as_list*)args);

  query_item * query_data = (query_item *) malloc ( sizeof(query_item) );
  if (! query_data) rb_raise(rb_aero_MemoryError, "[AerospikeC::Client][query] Error while allocating memory for query result");
  init_query_item(query_data);

  query_list q_args;

  q_args.as         = as;
  q_args.args       = args;
  q_args.policy     = policy;
  q_args.query      = query;
  q_args.query_data = query_data;
  q_args.callback   = execute_udf_on_query_callback;
  q_args.result     = rb_ary_new();

  VALUE result = rb_ensure(execute_udf_on_query_begin, (VALUE)(&q_args), execute_udf_on_query_ensure, (VALUE)(&q_args));

  VALUE mod_info = rb_sprintf("mod: %s, func: %s", c_module_name, c_func_name);

  log_info_with_time_v("[Client][aggregate]", &tm, mod_info);

  return result;
}

// ----------------------------------------------------------------------------------
//
// background execute udf on query
//
// def background_execute_udf_on_query(query_obj, module_name, func_name, udf_args = [])
//
// params:
//   query_obj - AeropsikeC::Query object
//   module_name - string, registered module name
//   func_name - string, function name in module to execute
//   udf_args - arguments passed to udf
//
//  ------
//  RETURN:
//    1. AerospikeC::QueryTask object
//
// @TODO options policy in AeropsikeC::Query
//
static VALUE background_execute_udf_on_query(int argc, VALUE * argv, VALUE self)  {
  struct timeval tm;
  start_timing(&tm);

  as_error err;
  aerospike * as = get_client_struct(self);

  VALUE query_obj;
  VALUE module_name;
  VALUE func_name;
  VALUE udf_args;

  rb_scan_args(argc, argv, "31", &query_obj, &module_name, &func_name, &udf_args);

  VALUE is_aerospike_c_query_obj = rb_funcall(query_obj, rb_intern("is_a?"), 1, rb_aero_Query);
  if ( is_aerospike_c_query_obj != Qtrue ) {
    rb_raise(rb_aero_OptionError, "[AerospikeC::Client][bg_aggregate] use AerospikeC::Query class to perform queries");
  }

  as_arraylist * args = NULL;
  if ( !(NIL_P(udf_args)) ) {
    args = array2as_list(udf_args);
  }

  as_query * query    = query_obj2as_query(query_obj);

  char * c_module_name = StringValueCStr(module_name);
  char * c_func_name = StringValueCStr(func_name);

  as_query_apply(query, c_module_name, c_func_name, (as_list*)args);

  uint64_t query_id = 0;

  if (aerospike_query_background(as, &err, NULL, query, &query_id) != AEROSPIKE_OK) {
    as_query_destroy(query);
    as_arraylist_destroy(args);
    raise_as_error(err);
  }

  as_arraylist_destroy(args);

  VALUE queryid  = ULONG2NUM(query_id);
  VALUE rb_query = Data_Wrap_Struct(rb_aero_Query, NULL, query_task_deallocate, query);

  rb_iv_set(self, "@last_query_id", queryid);

  VALUE mod_info = rb_sprintf("mod: %s, func: %s", c_module_name, c_func_name);

  log_info_with_time_v("[Client][bg_aggregate] success", &tm, mod_info);

  return rb_funcall(rb_aero_QueryTask, rb_intern("new"), 3, queryid, rb_query, self);
}


// ----------------------------------------------------------------------------------
//
// closes connection to the cluster
//
// def close
//
//  ------
//  RETURN:
//    1. self
//
static VALUE close_connection(VALUE self) {
  struct timeval tm;
  start_timing(&tm);

  as_error err;
  aerospike * as = get_client_struct(self);

  if ( aerospike_close(as, &err) != AEROSPIKE_OK )  {
    raise_as_error(err);
  }

  log_info_with_time("[Client][close] done!", &tm);

  return self;
}

// ----------------------------------------------------------------------------------
//
// creates new AerospikeC::Llist instance
//
// def llist(key, bin_name, options = {})
//
// params:
//   key - AerospikeC::Key object
//   bin_name - name of the bin to contain the ldt
//
//  ------
//  RETURN:
//    1. new AerospikeC::Llist object
//
static VALUE llist(int argc, VALUE * argv, VALUE self) {
  VALUE key;
  VALUE bin_name;
  VALUE options;

  rb_scan_args(argc, argv, "21", &key, &bin_name, &options);

  if ( NIL_P(options) ) options = rb_hash_new(); // default options

  log_info("[Client] Getting new Llist");

  return rb_funcall(rb_aero_Llist, rb_intern("new"), 4, self, key, bin_name, options);
}

// ----------------------------------------------------------------------------------
//
// Init
//
void init_aerospike_c_client(VALUE AerospikeC) {
  //
  // class AerospikeC::Client < Object
  //
  rb_aero_Client = rb_define_class_under(AerospikeC, "Client", rb_cObject);
  rb_define_alloc_func(rb_aero_Client, client_allocate);

  //
  // methods
  //
  rb_define_method(rb_aero_Client, "initialize", RB_FN_ANY()client_initialize, -1);
  rb_define_method(rb_aero_Client, "close", RB_FN_ANY()close_connection, 0);

  // crud
  rb_define_method(rb_aero_Client, "put", RB_FN_ANY()put, -1);
  rb_define_method(rb_aero_Client, "get", RB_FN_ANY()get, -1);
  rb_define_method(rb_aero_Client, "delete", RB_FN_ANY()delete_record, -1);

  // utils
  rb_define_method(rb_aero_Client, "logger=", RB_FN_ANY()set_logger, 1);
  rb_define_method(rb_aero_Client, "exists?", RB_FN_ANY()key_exists, -1);
  rb_define_method(rb_aero_Client, "get_header", RB_FN_ANY()get_header, -1);
  rb_define_method(rb_aero_Client, "batch_get", RB_FN_ANY()batch_get, -1);
  rb_define_method(rb_aero_Client, "touch", RB_FN_ANY()touch, -1);

  // operations
  rb_define_method(rb_aero_Client, "operate", RB_FN_ANY()operate, -1);
  rb_define_method(rb_aero_Client, "operation", RB_FN_ANY()operation_obj, 0);

  // indexes
  rb_define_method(rb_aero_Client, "create_index", RB_FN_ANY()create_index, -1);
  rb_define_method(rb_aero_Client, "drop_index", RB_FN_ANY()drop_index, -1);

  // info
  rb_define_method(rb_aero_Client, "info_cmd", RB_FN_ANY()info_cmd, 1);

  // udfs
  rb_define_method(rb_aero_Client, "register_udf", RB_FN_ANY()register_udf, -1);
  rb_define_method(rb_aero_Client, "drop_udf", RB_FN_ANY()drop_udf, -1);
  rb_define_method(rb_aero_Client, "list_udf", RB_FN_ANY()list_udf, -1);
  rb_define_method(rb_aero_Client, "execute_udf", RB_FN_ANY()execute_udf, -1);

  // scans
  rb_define_method(rb_aero_Client, "scan", RB_FN_ANY()scan_records, -1);
  rb_define_method(rb_aero_Client, "execute_udf_on_scan", RB_FN_ANY()execute_udf_on_scan, -1);
  rb_define_method(rb_aero_Client, "background_execute_udf_on_scan", RB_FN_ANY()background_execute_udf_on_scan, -1);

  // queries
  rb_define_method(rb_aero_Client, "query", RB_FN_ANY()execute_query, 1);
  rb_define_method(rb_aero_Client, "execute_udf_on_query", RB_FN_ANY()execute_udf_on_query, -1);
  rb_define_method(rb_aero_Client, "background_execute_udf_on_query", RB_FN_ANY()background_execute_udf_on_query, -1);

  // llist
  rb_define_method(rb_aero_Client, "llist", RB_FN_ANY()llist, -1);

  //
  // aliases
  //
  rb_define_alias(rb_aero_Client, "aggregate",    "execute_udf_on_query"           );
  rb_define_alias(rb_aero_Client, "bg_aggregate", "background_execute_udf_on_query");
  rb_define_alias(rb_aero_Client, "scan_udf",     "execute_udf_on_scan"            );
  rb_define_alias(rb_aero_Client, "bg_scan_udf",  "background_execute_udf_on_scan" );
  rb_define_alias(rb_aero_Client, "list_udfs",    "list_udf"                       );

  //
  // attr_reader
  //
  rb_define_attr(rb_aero_Client, "host", 1, 0);
  rb_define_attr(rb_aero_Client, "port", 1, 0);
  rb_define_attr(rb_aero_Client, "last_scan_id", 1, 0);
  rb_define_attr(rb_aero_Client, "last_query_id", 1, 0);
}