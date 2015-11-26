#include <aerospike_c_ruby.h>

VALUE Client;
VALUE Logger;

// ----------------------------------------------------------------------------------
//
// free memory method
//
static void client_deallocate(aerospike * as) {
  aerospike_destroy(as);
}

// ----------------------------------------------------------------------------------
//
// if with_header = true then add header to hash
//
static VALUE check_with_header(VALUE bins, VALUE options, as_record * rec) {
  if ( rb_hash_aref(options, with_header_sym) == Qtrue ) {
    VALUE header_hash = rb_hash_new();
    rb_hash_aset(header_hash, rb_str_new2("gen"), INT2FIX(rec->gen));
    rb_hash_aset(header_hash, rb_str_new2("expire_in"), INT2FIX(rec->ttl));

    if ( rb_hash_aref(bins, rb_str_new2("header")) != Qnil ) {
      rb_hash_aset(bins, rb_str_new2("record_header"), header_hash);
    }
    else {
      rb_hash_aset(bins, rb_str_new2("header"), header_hash);
    }
  }

  return bins;
}

//
// def initialize(host, port)
//
static void client_initialize(VALUE self, VALUE host, VALUE port) {
  rb_iv_set(self, "@host", host);
  rb_iv_set(self, "@port", port);

  as_config config;
  as_config_init(&config);
  as_config_add_host(&config, StringValueCStr(host), FIX2INT(port));

  aerospike * as = aerospike_new(&config);

  as_error err;
  if (aerospike_connect(as, &err) != AEROSPIKE_OK) {
    raise_as_error(err);
  }

  VALUE client_struct = Data_Wrap_Struct(Client, NULL, client_deallocate, as);

  rb_iv_set(self, "client", client_struct);
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
//
// @TODO options policy
//
static VALUE put(int argc, VALUE * argv, VALUE self) {
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
    if ( TYPE(rb_hash_aref(options, ttl_sym)) != T_FIXNUM ) { // check ttl option
      rb_raise(rb_eRuntimeError, "[AerospikeC::Client][put] ttl must be an integer");
    }
  }

  as_key * k = get_key_struct(key);

  VALUE new_rec;

  if ( TYPE(hash) == T_HASH ) {
    new_rec = rb_funcall(Record, rb_intern("new"), 1, hash);
  }
  else {
    new_rec = hash;
  }

  rec = get_record_struct(new_rec);
  rec->ttl = FIX2INT( rb_hash_aref(options, ttl_sym) );

  if (aerospike_key_put(as, &err, NULL, k, rec) != AEROSPIKE_OK) {
    raise_as_error(err);
  }

  as_record_destroy(rec);

  log_info("[AerospikeC::Client][put] success");

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
//
//  ------
//  RETURN:
//    1. hash representing record
//    2. nil when AEROSPIKE_ERR_RECORD_NOT_FOUND
//
// @TODO options policy
//
static VALUE get(int argc, VALUE * argv, VALUE self) {
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

  // read specific bins
  if ( specific_bins != Qnil ) {
    if ( TYPE(specific_bins) != T_ARRAY ) {
      rb_raise(rb_eRuntimeError, "[AerospikeC::Client][get] specific_bins must be an Array");
    }

    char ** inputArray = rb_array2inputArray(specific_bins); // convert ruby array to char **

    if ( ( status = aerospike_key_select(as, &err, NULL, k, inputArray, &rec) ) != AEROSPIKE_OK) {
      as_record_destroy(rec);
      inputArray_destroy(inputArray);

      if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
        log_warn("[AerospikeC::Client][get] AEROSPIKE_ERR_RECORD_NOT_FOUND");
        return Qnil;
      }

      raise_as_error(err);
    }

    bins = record2hash(rec);
    bins = check_with_header(bins, options, rec);

    as_record_destroy(rec);
    inputArray_destroy(inputArray);

    return bins;
  }

  // read all bins
  if ( ( status = aerospike_key_get(as, &err, NULL, k, &rec) ) != AEROSPIKE_OK) {
    as_record_destroy(rec);

    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[AerospikeC::Client][get] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    raise_as_error(err);
  }

  bins = record2hash(rec);
  bins = check_with_header(bins, options, rec);

  as_record_destroy(rec);

  log_info("[AerospikeC::Client][get] success");

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
//
//  ------
//  RETURN:
//    1. true if deleted
//    2. nil if AEROSPIKE_ERR_RECORD_NOT_FOUND
//
static VALUE delete_record(VALUE self, VALUE key) {
  as_error err;
  as_status status;
  as_key * k     = get_key_struct(key);
  aerospike * as = get_client_struct(self);

  if ( ( status = aerospike_key_remove(as, &err, NULL, k) ) != AEROSPIKE_OK ) {
    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[AerospikeC::Client][delete] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    raise_as_error(err);
  }

  log_info("[AerospikeC::Client][delete] success");

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
  Logger = logger;
  return Qtrue;
}

//
// def exists?(key)
//
// params:
//   key - AerospikeC::Key object
//
//  ------
//  RETURN:
//    1. true if exist
//    2. false otherwise
//
static VALUE key_exists(VALUE self, VALUE key) {
  as_error err;
  as_status status;
  aerospike * as  = get_client_struct(self);
  as_key * k      = get_key_struct(key);
  as_record * rec = NULL;

  if ( ( status = aerospike_key_exists(as, &err, NULL, k, &rec) ) != AEROSPIKE_OK ) {
    as_record_destroy(rec);

    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      return Qfalse;
    }

    raise_as_error(err);
  }
  else {
    as_record_destroy(rec);
    return Qtrue;
  }
}

// ----------------------------------------------------------------------------------
//
// def get_header(key)
//
// params:
//   key - AerospikeC::Key object
//
//  ------
//  RETURN:
//    1. hash representing record header
//    2. nil when AEROSPIKE_ERR_RECORD_NOT_FOUND
//
static VALUE get_header(VALUE self, VALUE key) {
  as_error err;
  as_status status;
  aerospike * as  = get_client_struct(self);
  as_key * k      = get_key_struct(key);
  as_record * rec = NULL;

  VALUE header = rb_hash_new();

  const char * inputArray[] = { NULL };

  if ( ( status = aerospike_key_select(as, &err, NULL, k, inputArray, &rec) ) != AEROSPIKE_OK) {
    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[AerospikeC::Client][get_header] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    raise_as_error(err);
  }

  rb_hash_aset(header, rb_str_new2("gen"), INT2FIX(rec->gen));
  rb_hash_aset(header, rb_str_new2("expire_in"), INT2FIX(rec->ttl));

  as_record_destroy(rec);

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
    if ( TYPE(specific_bins) != T_ARRAY ) rb_raise(rb_eRuntimeError, "[AerospikeC::Client][batch_get] specific_bins must be an Array");

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

    tmp = rb_funcall(element, rb_intern("key"), 0);
    char * c_key = StringValueCStr( tmp );

    as_batch_read_record * record = as_batch_read_reserve(&records);
    as_key_init(&record->key, c_namespace, c_set, c_key);

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
      log_warn("[AerospikeC::Client][batch_get] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

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

  log_debug("[AerospikeC::Client][batch_get] success");

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
//
//  ------
//  RETURN:
//    1. hash representing record header
//    2. nil when AEROSPIKE_ERR_RECORD_NOT_FOUND
//
// @TODO options policy
//
static VALUE touch(int argc, VALUE * argv, VALUE self) {
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
      rb_raise(rb_eRuntimeError, "[AerospikeC::Client][put] ttl must be an integer");
    }
  }

  as_key * k = get_key_struct(key);
  as_record * rec = NULL;

  as_operations ops;
  as_operations_inita(&ops, 1);
  as_operations_add_touch(&ops);

  ops.ttl = FIX2INT( rb_hash_aref(options, ttl_sym) );

  if ( ( status = aerospike_key_operate(as, &err, NULL, k, &ops, &rec) ) != AEROSPIKE_OK ) {
    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[AerospikeC::Client][touch] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    raise_as_error(err);
  }

  VALUE header = rb_hash_new();

  rb_hash_aset(header, rb_str_new2("gen"), INT2FIX(rec->gen));
  rb_hash_aset(header, rb_str_new2("expire_in"), INT2FIX(rec->ttl));

  as_record_destroy(rec);
  as_operations_destroy(&ops);

  return header;
}

// ----------------------------------------------------------------------------------
//
// perform given operations on record in one call
//
// def operate(key, operations)
//
// params:
//   key - AeropsikeC::Key object
//   operations - AeropsikeC::Operation object
//
//  ------
//  RETURN:
//    1. hash representing record
//    2. nil when AEROSPIKE_ERR_RECORD_NOT_FOUND
//
// @TODO options policy
//
static VALUE operate(VALUE self, VALUE key, VALUE operations) {
  as_error err;
  as_status status;
  aerospike * as = get_client_struct(self);
  as_key * k     = get_key_struct(key);

  VALUE is_aerospike_c_operation = rb_funcall(operations, rb_intern("is_a?"), 1, Operation);
  if ( is_aerospike_c_operation != Qtrue ) {
    rb_raise(rb_eRuntimeError, "[AerospikeC::Client][operate] use AerospikeC::Operation class to perform operations");
  }

  VALUE rb_ops = rb_iv_get(operations, "@operations");

  int ops_count = rb_ary_len_int(rb_ops);

  as_operations ops;
  as_operations_inita(&ops, ops_count);
  ops.ttl = FIX2INT( rb_iv_get(operations, "@ttl") );

  for (int i = 0; i < ops_count; ++i) {
    VALUE op = rb_ary_entry(rb_ops, i);

    VALUE rb_bin = rb_hash_aref(op, bin_sym);
    char * bin_name = StringValueCStr(rb_bin);

    VALUE val = rb_hash_aref(op, value_sym);
    VALUE operation_type = rb_hash_aref(op, operation_sym);

    if ( operation_type == touch_sym ) {
      as_operations_add_touch(&ops);
    }
    else if ( operation_type == read_sym ) {
      as_operations_add_read(&ops, bin_name);
    }
    else if ( operation_type == increment_sym ) {
      as_operations_add_incr(&ops, bin_name, FIX2LONG(val));
    }
    else if ( operation_type == append_sym ) {
      as_operations_add_append_str(&ops, bin_name, StringValueCStr(val));
    }
    else if ( operation_type == prepend_sym ) {
      as_operations_add_prepend_str(&ops, bin_name, StringValueCStr(val));
    }
    else if ( operation_type == write_sym ) {
      if ( TYPE(val) == T_FIXNUM ) {
        as_operations_add_write_int64(&ops, bin_name, FIX2LONG(val));
      }
      else if ( TYPE(val) == T_STRING ) {
        as_operations_add_write_str(&ops, bin_name, StringValueCStr(val));
      }
      else {
        VALUE tmp = value_to_s(val);
        as_operations_add_write_str(&ops, bin_name, StringValueCStr(tmp));
      }
    }
    else {
      rb_raise(rb_eRuntimeError, "[AerospikeC::Client][operate] uknown operation type");
    }
  }

  as_record * rec = NULL;

  if ( ( status = aerospike_key_operate(as, &err, NULL, k, &ops, &rec) ) != AEROSPIKE_OK ) {
    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[AerospikeC::Client][touch] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    raise_as_error(err);
  }

  VALUE record = record2hash(rec);

  as_record_destroy(rec);
  as_operations_destroy(&ops);

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
  return rb_funcall(Operation, rb_intern("new"), 0);
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
  if ( NIL_P(options) ) {
    options = rb_hash_new();
  }

  int d_type;

  if ( data_type == numeric_sym ) {
    d_type = AS_INDEX_NUMERIC;
  }
  else if ( data_type == string_sym ) {
    d_type = AS_INDEX_STRING;
  }
  else {
    rb_raise(rb_eRuntimeError, "[AerospikeC::Client][create_index] data_type must be :string or :numeric");
  }


  as_index_task * task = (as_index_task *) malloc( sizeof(as_index_task) );

  if ( aerospike_index_create(as, &err, task, NULL, StringValueCStr(ns), StringValueCStr(set),
                              StringValueCStr(bin), StringValueCStr(name), d_type) != AEROSPIKE_OK ) {
    raise_as_error(err);
  }

  VALUE index_task_struct = Data_Wrap_Struct(IndexTask, NULL, index_task_deallocate, task);

  return rb_funcall(IndexTask, rb_intern("new"), 1, index_task_struct);
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
  as_error err;
  aerospike * as = get_client_struct(self);

  VALUE ns;
  VALUE name;
  VALUE options;

  rb_scan_args(argc, argv, "21", &ns, &name, &options);

  // default values for optional arguments
  if ( NIL_P(options) ) {
    options = rb_hash_new();
  }

  if ( aerospike_index_remove(as, &err, NULL, StringValueCStr(ns), StringValueCStr(name)) != AEROSPIKE_OK ) {
    raise_as_error(err);
  }

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

  return info_res;
}

// ----------------------------------------------------------------------------------
//
// Init
//
void init_aerospike_c_client(VALUE AerospikeC) {
  //
  // class AerospikeC::Client < Object
  //
  Client = rb_define_class_under(AerospikeC, "Client", rb_cObject);

  //
  // methods
  //
  rb_define_method(Client, "initialize", RB_FN_ANY()client_initialize, 2);

  rb_define_method(Client, "put", RB_FN_ANY()put, -1);
  rb_define_method(Client, "get", RB_FN_ANY()get, -1);
  rb_define_method(Client, "delete", RB_FN_ANY()delete_record, 1);

  rb_define_method(Client, "logger=", RB_FN_ANY()set_logger, 1);
  rb_define_method(Client, "exists?", RB_FN_ANY()key_exists, 1);
  rb_define_method(Client, "get_header", RB_FN_ANY()get_header, 1);
  rb_define_method(Client, "batch_get", RB_FN_ANY()batch_get, -1);
  rb_define_method(Client, "touch", RB_FN_ANY()touch, -1);

  rb_define_method(Client, "operate", RB_FN_ANY()operate, 2);
  rb_define_method(Client, "operation", RB_FN_ANY()operation_obj, 0);

  rb_define_method(Client, "create_index", RB_FN_ANY()create_index, -1);
  rb_define_method(Client, "drop_index", RB_FN_ANY()drop_index, -1);
  rb_define_method(Client, "info_cmd", RB_FN_ANY()info_cmd, 1);

  //
  // attr_reader
  //
  rb_define_attr(Client, "host", 1, 0);
  rb_define_attr(Client, "port", 1, 0);
}