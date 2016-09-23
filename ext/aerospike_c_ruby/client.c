#include <aerospike_c_ruby.h>
#include <client_utils.h>
#include <scan.h>
#include <querying.h>

VALUE rb_aero_Client;


//
// Aerospike queries and scans callback runs in parallel
// Better not touch ruby api inside threads...
//
pthread_mutex_t G_CALLBACK_MUTEX = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

//
// client allocate
//
static VALUE client_allocate(VALUE self) {
  aerospike * as = (aerospike *) malloc ( sizeof(aerospike) );

  return Data_Wrap_Struct(self, NULL, client_deallocate, as);
}


// ----------------------------------------------------------------------------------
//
// def initialize(host, port, options = {})
//
static void client_initialize(int argc, VALUE * argv, VALUE self) {
  rb_aero_TIMED(tm);

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

  as_log_set_level(AS_LOG_LEVEL_DEBUG);

  VALUE option_tmp = rb_hash_aref(options, c_log_sym);

  if ( TYPE(option_tmp) == T_TRUE ) {
    as_log_set_callback(rb_aero_log_callback);
  }
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
  rb_aero_TIMED(tm);

  as_status status;
  as_error err;
  aerospike * as  = rb_aero_CLIENT;

  VALUE key;
  VALUE hash;
  VALUE options;

  rb_scan_args(argc, argv, "21", &key, &hash, &options);

  if ( NIL_P(options) ) options = rb_hash_new();

  VALUE option_tmp;


  long len = rb_ary_len_long(hash);

  as_record rec;
  as_record_init(&rec, len);

  hash2record(hash, &rec);

  as_key * k = rb_aero_KEY;
  as_policy_write * policy = get_policy(options);

  if ( !NIL_P(options) ) {
    VALUE rb_ttl = rb_hash_aref(options, ttl_sym);

    if ( TYPE(rb_ttl) == T_FIXNUM ) {
      rec.ttl = FIX2INT( rb_ttl );
    }
  }


  if ( ( status = aerospike_key_put(as, &err, policy, k, &rec) ) != AEROSPIKE_OK) {
    as_record_destroy(&rec);

    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      rb_aero_logger(AS_LOG_LEVEL_WARN, &tm, 2, rb_str_new2("[Client][put] AEROSPIKE_ERR_RECORD_NOT_FOUND"), rb_aero_KEY_INFO);
      return Qnil;
    }

    raise_as_error(err);
  }

  as_record_destroy(&rec);

  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 2, rb_str_new2("[Client][put] success"), rb_aero_KEY_INFO);
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
  rb_aero_TIMED(tm);

  as_error err;
  as_status status;
  aerospike * as  = rb_aero_CLIENT;
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

  as_key * k = rb_aero_KEY;
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
        rb_aero_logger(AS_LOG_LEVEL_WARN, &tm, 2, rb_str_new2("[Client][get] AEROSPIKE_ERR_RECORD_NOT_FOUND"), rb_aero_KEY_INFO);
        return Qnil;
      }

      raise_as_error(err);
    }

    bins = record2hash(rec);
    bins = check_with_header(bins, options, rec);

    as_record_destroy(rec);
    inputArray_destroy(inputArray);

    check_for_llist_workaround(self, key, bins);

    rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 2, rb_str_new2("[Client][get] success"), rb_aero_KEY_INFO);

    return bins;
  }

  // read all bins
  if ( ( status = aerospike_key_get(as, &err, policy, k, &rec) ) != AEROSPIKE_OK) {
    as_record_destroy(rec);

    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      rb_aero_logger(AS_LOG_LEVEL_WARN, &tm, 2, rb_str_new2("[Client][get] AEROSPIKE_ERR_RECORD_NOT_FOUND"), rb_aero_KEY_INFO);
      return Qnil;
    }

    raise_as_error(err);
  }

  bins = record2hash(rec);
  bins = check_with_header(bins, options, rec);

  as_record_destroy(rec);

  check_for_llist_workaround(self, key, bins);

  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 2, rb_str_new2("[Client][get] success"), rb_aero_KEY_INFO);

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
  rb_aero_TIMED(tm);

  as_error err;
  as_status status;
  aerospike * as = rb_aero_CLIENT;

  VALUE key;
  VALUE options;

  rb_scan_args(argc, argv, "11", &key, &options);

  if ( NIL_P(options) ) options = rb_hash_new();

  as_key * k = rb_aero_KEY;
  as_policy_remove * policy = get_policy(options);

  if ( ( status = aerospike_key_remove(as, &err, policy, k) ) != AEROSPIKE_OK ) {
    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      rb_aero_logger(AS_LOG_LEVEL_WARN, &tm, 2, rb_str_new2("AEROSPIKE_ERR_RECORD_NOT_FOUND"), rb_aero_KEY_INFO);
      return Qnil;
    }

    raise_as_error(err);
  }

  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 2, rb_str_new2("[Client][delete] success"), rb_aero_KEY_INFO);

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
static VALUE set_logger(VALUE self, VALUE logger_obj) {
  rb_funcall(rb_aero_AerospikeC, rb_intern("logger="), 1, logger_obj);
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
  rb_aero_TIMED(tm);

  as_error err;
  as_status status;
  aerospike * as  = rb_aero_CLIENT;
  as_record * rec = NULL;

  VALUE key;
  VALUE options;

  rb_scan_args(argc, argv, "11", &key, &options);

  if ( NIL_P(options) ) options = rb_hash_new();

  as_key * k = rb_aero_KEY;
  as_policy_read * policy = get_policy(options);

  if ( ( status = aerospike_key_exists(as, &err, policy, k, &rec) ) != AEROSPIKE_OK ) {
    as_record_destroy(rec);

    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 2, rb_str_new2("[Client][exists?] success - false"), rb_aero_KEY_INFO);
      return Qfalse;
    }

    raise_as_error(err);
  }

  as_record_destroy(rec);
  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 2, rb_str_new2("[Client][exists?] success - true"), rb_aero_KEY_INFO);
  return Qtrue;
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
  rb_aero_TIMED(tm);

  as_error err;
  as_status status;
  aerospike * as  = rb_aero_CLIENT;
  as_record * rec = NULL;

  VALUE key;
  VALUE options;

  rb_scan_args(argc, argv, "11", &key, &options);

  if ( NIL_P(options) ) options = rb_hash_new();

  as_key * k = rb_aero_KEY;
  as_policy_read * policy = get_policy(options);

  if ( ( status = aerospike_key_exists(as, &err, policy, k, &rec) ) != AEROSPIKE_OK ) {
    as_record_destroy(rec);

    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      rb_aero_logger(AS_LOG_LEVEL_WARN, &tm, 2, rb_str_new2("[Client][get_header] AEROSPIKE_ERR_RECORD_NOT_FOUND"), rb_aero_KEY_INFO);
      return Qnil;
    }

    raise_as_error(err);
  }

  VALUE header = rb_hash_new();

  rb_hash_aset(header, rb_str_new2("gen"), INT2FIX(rec->gen));
  rb_hash_aset(header, rb_funcall(rb_aero_AerospikeC, rb_intern("ttl_name"), 0), INT2FIX(rec->ttl));

  as_record_destroy(rec);

  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 2, rb_str_new2("[Client][get_header] success"), rb_aero_KEY_INFO);

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
  rb_aero_TIMED(tm);

  as_error err;
  as_status status;
  aerospike * as = rb_aero_CLIENT;
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
      rb_aero_logger(AS_LOG_LEVEL_WARN, &tm, 1, rb_str_new2("[Client][batch_get] AEROSPIKE_ERR_RECORD_NOT_FOUND"));
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

  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 1, rb_str_new2("[Client][batch_get] success"));

  return records_bins;
}


/**
 * @brief      Callback for batch_exists
 *
 * @param[in]  results  The results
 * @param[in]  n        number for resulsts
 * @param      udata    The user data
 *
 * @return     true on success. Otherwise, an error occurred
 */
static bool batch_exists_callback(const as_batch_read * results, uint32_t n, VALUE return_data) {
  for (uint32_t i = 0; i < n; ++i) {
    if ( results[i].result != AEROSPIKE_OK ) continue;

    as_val * rec_val = (as_val *) results[i].key->valuep;

    VALUE ns  = rb_str_new2(results[i].key->ns);
    VALUE set = rb_str_new2(results[i].key->set);
    VALUE val;

    switch ( as_val_type(rec_val) ) {
      case AS_INTEGER: {
        val = as_val_int_2_val(rec_val);
        break;
      }

      case AS_STRING: {
        val = as_val_str_2_val(rec_val);
        break;
      }
    }

    VALUE key;

    key = rb_funcall(rb_aero_Key, rb_intern("new"), 3, ns, set, val);

    rb_hash_aset(return_data, key, Qtrue);
  }

  return true;
}


/**
 * @brief      Test whether multiple records exist in the cluster.
 *
 * @param[in]  argc  The argc
 * @param      argv  The argv
 * @param[in]  self  The object
 *
 * @return     { description_of_the_return_value }
 */
static VALUE batch_exists(int argc, VALUE * argv, VALUE self) {
  rb_aero_TIMED(tm);

  as_error err;
  as_status status;
  aerospike * as = rb_aero_CLIENT;

  VALUE keys;
  VALUE options;

  rb_scan_args(argc, argv, "11", &keys, &options);

  long keys_len = rb_ary_len_long(keys);

  as_batch batch;
  as_batch_inita(&batch, keys_len);

  VALUE return_data = rb_hash_new();

  for (long i = 0; i < keys_len; ++i) {
    VALUE element = rb_ary_entry(keys, i);

    // set into hash for return values
    rb_hash_aset(return_data, element, Qfalse);

    VALUE tmp = rb_funcall(element, rb_intern("namespace"), 0);
    char * c_namespace = StringValueCStr( tmp );

    tmp = rb_funcall(element, rb_intern("set"), 0);
    char * c_set = StringValueCStr( tmp );

    tmp = rb_funcall(element, rb_intern("key"), 0);

    if ( TYPE(tmp) != T_FIXNUM ) {
      char * c_key = StringValueCStr( tmp );
      as_key_init(as_batch_keyat(&batch,i), c_namespace, c_set, c_key);
    }
    else {
      as_key_init_int64(as_batch_keyat(&batch,i), c_namespace, c_set, FIX2LONG(tmp));
    }
  }

  if ( aerospike_batch_exists(as, &err, NULL, &batch, batch_exists_callback, return_data) != AEROSPIKE_OK ) {
    as_batch_destroy(&batch);
    raise_as_error(err);
  }

  as_batch_destroy(&batch);

  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 1, rb_str_new2("[Client][batch_exists] success"));

  return return_data;
}


/**
 * @brief      Callback for batch_exists
 *
 * @param[in]  results  The results
 * @param[in]  n        number for resulsts
 * @param      udata    The user data
 *
 * @return     true on success. Otherwise, an error occurred
 */
static bool batch_read_callback(const as_batch_read * results, uint32_t n, VALUE return_data) {
  for (uint32_t i = 0; i < n; ++i) {
    as_val * rec_val = (as_val *) results[i].key->valuep;

    VALUE ns  = rb_str_new2(results[i].key->ns);
    VALUE set = rb_str_new2(results[i].key->set);
    VALUE val;

    switch ( as_val_type(rec_val) ) {
      case AS_INTEGER: {
        val = as_val_int_2_val(rec_val);
        break;
      }

      case AS_STRING: {
        val = as_val_str_2_val(rec_val);
        break;
      }
    }

    VALUE key = rb_funcall(rb_aero_Key, rb_intern("new"), 3, ns, set, val);

    if ( results[i].result == AEROSPIKE_OK ) {
      rb_hash_aset(return_data, key, record2hash(&results[i].record));
    }
    else if ( results[i].result == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      rb_hash_aset(return_data, key, Qnil);
    }
    else {
      VALUE ErrCodes = rb_const_get(rb_cObject, rb_intern("AerospikeC::ErrorCodes"));
      rb_hash_aset(return_data, key, rb_funcall(ErrCodes, rb_intern("new"), 1, INT2FIX(results[i].result)));
    }
  }

  return true;
}


/**
 * @brief      Test whether multiple records exist in the cluster.
 *
 * @param[in]  argc  The argc
 * @param      argv  The argv
 * @param[in]  self  The object
 *
 * @return     { description_of_the_return_value }
 */
static VALUE batch_read(int argc, VALUE * argv, VALUE self) {
  rb_aero_TIMED(tm);

  as_error err;
  as_status status;
  aerospike * as = rb_aero_CLIENT;

  VALUE keys;
  VALUE options;

  rb_scan_args(argc, argv, "11", &keys, &options);

  long keys_len = rb_ary_len_long(keys);

  as_batch batch;
  as_batch_inita(&batch, keys_len);

  VALUE return_data = rb_hash_new();

  for (long i = 0; i < keys_len; ++i) {
    VALUE element = rb_ary_entry(keys, i);

    // set into hash for return values
    rb_hash_aset(return_data, element, Qfalse);

    VALUE tmp = rb_funcall(element, rb_intern("namespace"), 0);
    char * c_namespace = StringValueCStr( tmp );

    tmp = rb_funcall(element, rb_intern("set"), 0);
    char * c_set = StringValueCStr( tmp );

    tmp = rb_funcall(element, rb_intern("key"), 0);

    if ( TYPE(tmp) != T_FIXNUM ) {
      char * c_key = StringValueCStr( tmp );
      as_key_init(as_batch_keyat(&batch,i), c_namespace, c_set, c_key);
    }
    else {
      as_key_init_int64(as_batch_keyat(&batch,i), c_namespace, c_set, FIX2LONG(tmp));
    }
  }

  if ( aerospike_batch_get(as, &err, NULL, &batch, batch_read_callback, return_data) != AEROSPIKE_OK ) {
    as_batch_destroy(&batch);
    raise_as_error(err);
  }

  as_batch_destroy(&batch);

  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 1, rb_str_new2("[Client][batch_read] success"));

  return return_data;
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
  rb_aero_TIMED(tm);

  as_error err;
  as_status status;
  aerospike * as = rb_aero_CLIENT;

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

  as_key * k = rb_aero_KEY;
  as_record * rec = NULL;

  as_operations ops;
  as_operations_inita(&ops, 1);
  as_operations_add_touch(&ops);

  ops.ttl = FIX2INT( rb_hash_aref(options, ttl_sym) );

  as_policy_operate * policy = get_policy(options);

  if ( ( status = aerospike_key_operate(as, &err, policy, k, &ops, &rec) ) != AEROSPIKE_OK ) {
    as_operations_destroy(&ops);

    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      rb_aero_logger(AS_LOG_LEVEL_WARN, &tm, 2, rb_str_new2("[Client][touch] AEROSPIKE_ERR_RECORD_NOT_FOUND"), rb_aero_KEY_INFO);
      return Qnil;
    }

    raise_as_error(err);
  }

  VALUE header = rb_hash_new();

  rb_hash_aset(header, rb_str_new2("gen"), INT2FIX(rec->gen));
  rb_hash_aset(header, rb_str_new2("expire_in"), INT2FIX(rec->ttl));

  as_record_destroy(rec);
  as_operations_destroy(&ops);

  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 2, rb_str_new2("[Client][touch] success"), rb_aero_KEY_INFO);

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
  rb_aero_TIMED(tm);

  as_error err;
  as_status status;
  aerospike * as = rb_aero_CLIENT;

  VALUE key;
  VALUE operations;
  VALUE options;

  rb_scan_args(argc, argv, "21", &key, &operations, &options);

  if ( NIL_P(options) ) {
    options = rb_hash_new();
  }

  as_key * k     = rb_aero_KEY;

  VALUE is_aerospike_c_operation = rb_funcall(operations, rb_intern("is_a?"), 1, rb_aero_OPERATION);
  if ( is_aerospike_c_operation != Qtrue ) {
    rb_raise(rb_aero_OptionError, "[AerospikeC::Client][operate] use AerospikeC::Operation class to perform operations");
  }

  as_operations * ops = rb_operations2as_operations(operations);

  as_record * rec = NULL;
  as_policy_operate * policy = get_policy(options);

  if ( ( status = aerospike_key_operate(as, &err, policy, k, ops, &rec) ) != AEROSPIKE_OK ) {
    as_operations_destroy(ops);

    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      rb_aero_logger(AS_LOG_LEVEL_WARN, &tm, 2, rb_str_new2("[Client][operate] AEROSPIKE_ERR_RECORD_NOT_FOUND"), rb_aero_KEY_INFO);
      return Qnil;
    }
    raise_as_error(err);
  }

  VALUE record = record2hash(rec);

  as_record_destroy(rec);
  as_operations_destroy(ops);

  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 2, rb_str_new2("[Client][operate] success"), rb_aero_KEY_INFO);

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
  return rb_funcall(rb_aero_OPERATION, rb_intern("new"), 0);
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
  rb_aero_TIMED(tm);

  as_error err;
  aerospike * as = rb_aero_CLIENT;

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

  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 1, rb_str_new2("[Client][create_index] done"));

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
  rb_aero_TIMED(tm);

  as_error err;
  aerospike * as = rb_aero_CLIENT;

  VALUE ns;
  VALUE name;
  VALUE options;

  rb_scan_args(argc, argv, "21", &ns, &name, &options);

  // default values for optional arguments
  if ( NIL_P(options) ) options = rb_hash_new();

  if ( aerospike_index_remove(as, &err, NULL, StringValueCStr(ns), StringValueCStr(name)) != AEROSPIKE_OK )
    raise_as_error(err);

  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 1, rb_str_new2("[Client][drop_index] done"));

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
  rb_aero_TIMED(tm);

  as_error err;
  aerospike * as = rb_aero_CLIENT;

  char * res = NULL;

  VALUE host = rb_iv_get(self, "@host");
  VALUE port = rb_iv_get(self, "@port");

  if ( aerospike_info_host(as, &err, NULL, StringValueCStr(host), FIX2INT(port), StringValueCStr(cmd), &res) != AEROSPIKE_OK ) {
    raise_as_error(err);
  }

  VALUE info_res = rb_str_new2(res);
  free(res);

  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 1, rb_str_new2("[Client][info_cmd] done"));

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
  rb_aero_TIMED(tm);

  as_error err;
  aerospike * as = rb_aero_CLIENT;

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

  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 1, rb_str_new2("[Client][register_udf] done"));

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
  rb_aero_TIMED(tm);

  as_error err;
  aerospike * as = rb_aero_CLIENT;

  VALUE server_path;
  VALUE options;

  rb_scan_args(argc, argv, "11", &server_path, &options);

  // default values for optional arguments
  if ( NIL_P(options) ) options = rb_hash_new();

  if ( aerospike_udf_remove(as, &err, NULL, StringValueCStr(server_path)) != AEROSPIKE_OK )
    raise_as_error(err);

  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 1, rb_str_new2("[Client][drop_udf] done"));

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
  aerospike * as = rb_aero_CLIENT;

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
  rb_aero_TIMED(tm);

  as_error err;
  as_status status;
  aerospike * as = rb_aero_CLIENT;

  VALUE key;
  VALUE module_name;
  VALUE func_name;
  VALUE udf_args;
  VALUE options;

  rb_scan_args(argc, argv, "32", &key, &module_name, &func_name, &udf_args, &options);

  // default values for optional arguments
  if ( NIL_P(options) ) options = rb_hash_new();
  if ( NIL_P(udf_args) ) udf_args = rb_ary_new();

  as_key * k               = rb_aero_KEY;
  as_arraylist * args      = array2as_list(udf_args);
  as_policy_apply * policy = get_policy(options);

  as_val * res = NULL;

  char * c_module_name = StringValueCStr(module_name);
  char * c_func_name   = StringValueCStr(func_name);

  if ( ( status = aerospike_key_apply(as, &err, policy, k, c_module_name, c_func_name, (as_list *)args, &res) ) != AEROSPIKE_OK ) {
    as_arraylist_destroy(args);

    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      rb_aero_logger(AS_LOG_LEVEL_WARN, &tm, 3, rb_str_new2("[Client][execute_udf] AEROSPIKE_ERR_RECORD_NOT_FOUND"), rb_aero_KEY_INFO, rb_aero_MOD_INFO);
      return Qnil;
    }

    raise_as_error(err);
  }

  VALUE result = as_val2rb_val(res);

  as_val_destroy(&res);
  as_arraylist_destroy(args);

  VALUE key_info = rb_aero_KEY_INFO;

  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 3, rb_str_new2("[Client][execute_udf] success"), rb_aero_KEY_INFO, rb_aero_MOD_INFO);

  return result;
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
  rb_aero_TIMED(tm);

  as_error err;
  aerospike * as = rb_aero_CLIENT;

  if ( aerospike_close(as, &err) != AEROSPIKE_OK )  {
    raise_as_error(err);
  }

  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 1, rb_str_new2("[Client][close] success"));

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
  rb_define_method(rb_aero_Client, "touch", RB_FN_ANY()touch, -1);

  // batch
  rb_define_method(rb_aero_Client, "batch_get", RB_FN_ANY()batch_get, -1);
  rb_define_method(rb_aero_Client, "batch_exists", RB_FN_ANY()batch_exists, -1);
  rb_define_method(rb_aero_Client, "batch_read", RB_FN_ANY()batch_read, -1);

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
  rb_define_method(rb_aero_Client, "scan_each", RB_FN_ANY()scan_each, -1);
  rb_define_method(rb_aero_Client, "execute_udf_on_scan", RB_FN_ANY()execute_udf_on_scan, -1);
  rb_define_method(rb_aero_Client, "background_execute_udf_on_scan", RB_FN_ANY()background_execute_udf_on_scan, -1);

  // queries
  rb_define_method(rb_aero_Client, "_query", RB_FN_ANY()execute_query, 1);
  rb_define_method(rb_aero_Client, "query_each", RB_FN_ANY()execute_query_each, 1);
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
