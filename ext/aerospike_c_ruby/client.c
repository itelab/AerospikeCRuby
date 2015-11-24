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
// def put(key, bins)
//
static VALUE put(VALUE self, VALUE key, VALUE hash) {
  as_error err;
  as_record * rec = NULL;

  as_key * k      = get_key_struct(key);
  aerospike * as  = get_client_struct(self);

  if ( TYPE(hash) != T_HASH ) {
    rb_raise(rb_eRuntimeError, "Bins must be a Hash");
  }

  VALUE new_rec = rb_funcall(Record, rb_intern("new"), 1, hash);
  rec = get_record_struct(new_rec);

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
// @TODO options
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

  if ( NIL_P(specific_bins) ) specific_bins = Qnil;
  if ( NIL_P(options) ) options = rb_hash_new();

  as_key * k = get_key_struct(key);

  // read specific bins
  if ( specific_bins != Qnil ) {
    if ( TYPE(specific_bins) != T_ARRAY ) {
      rb_raise(rb_eRuntimeError, "[AerospikeC::Client][get] specific_bins must be an Array");
    }

    char ** inputArray = rb_array2inputArray(specific_bins);

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
  as_record * rec = NULL;
  as_key * k     = get_key_struct(key);
  aerospike * as = get_client_struct(self);

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
  rb_define_method(Client, "put", RB_FN_ANY()put, 2);
  rb_define_method(Client, "get", RB_FN_ANY()get, -1);
  rb_define_method(Client, "delete", RB_FN_ANY()delete_record, 1);
  rb_define_method(Client, "logger=", RB_FN_ANY()set_logger, 1);
  rb_define_method(Client, "exists?", RB_FN_ANY()key_exists, 1);

  //
  // attr_reader
  //
  rb_define_attr(Client, "host", 1, 0);
  rb_define_attr(Client, "port", 1, 0);
}