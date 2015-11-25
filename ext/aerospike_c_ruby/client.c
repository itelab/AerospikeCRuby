#include <aerospike_c_ruby.h>

VALUE Client;
VALUE Logger;

//
// free memory method
//
static void client_deallocate(aerospike * as) {
  aerospike_destroy(as);
}

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

//
// def put(key, bins, options = {})
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

//
// def get(key, specific_bins = nil, options = {})
//
// specific bins should be an Array of bins
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

//
// def delete(key)
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

//
// def logger=(logger)
//
static VALUE set_logger(VALUE self, VALUE logger) {
  Logger = logger;
  return Qtrue;
}

//
// def exists?(key)
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

//
// def get_header(key)
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

//
// def batch_get(keys, specific_bins = nil, options = {})
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

  if ( ( status = aerospike_batch_read(as, &err, NULL, &records) ) != AEROSPIKE_OK ) {
    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[AerospikeC::Client][batch_get] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    raise_as_error(err);
  }

  as_vector list = records.list;

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

  //
  // attr_reader
  //
  rb_define_attr(Client, "host", 1, 0);
  rb_define_attr(Client, "port", 1, 0);
}