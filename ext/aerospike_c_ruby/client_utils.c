#include <client_utils.h>

// ----------------------------------------------------------------------------------
//
// free memory method
//
void client_deallocate(aerospike * as) {
  aerospike_destroy(as);
}

// ----------------------------------------------------------------------------------
//
// if with_header = true then add header to hash
//
VALUE check_with_header(VALUE bins, VALUE options, as_record * rec) {
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

// ----------------------------------------------------------------------------------
//
// init config with options
//
void options2config(as_config * config, VALUE options) {
  VALUE option_tmp = rb_hash_aref(options, hosts_sym);
  if ( option_tmp != Qnil ) { // hosts
    if ( TYPE(option_tmp) != T_ARRAY )
      rb_raise(rb_eRuntimeError, "[AerospikeC::Client][initialize] options :hosts must be an array");

    rb_foreach_ary_int(option_tmp) {
      VALUE host_info = rb_ary_entry(option_tmp, i);

      VALUE host = rb_hash_aref(host_info, host_sym);
      VALUE port = rb_hash_aref(host_info, port_sym);

      as_config_add_host(config, StringValueCStr(host), FIX2INT(port));
    }
  }

  option_tmp = rb_hash_aref(options, lua_path_sym);
  if ( option_tmp != Qnil ) { // lua_path
    if ( TYPE(option_tmp) != T_STRING )
      rb_raise(rb_eRuntimeError, "[AerospikeC::Client][initialize] options :lua_path must be string");

    strcpy(config->lua.user_path, StringValueCStr(option_tmp));
  }

  option_tmp = rb_hash_aref(options, password_sym);
  if ( option_tmp != Qnil ) { // password
    if ( TYPE(option_tmp) != T_STRING )
      rb_raise(rb_eRuntimeError, "[AerospikeC::Client][initialize] options :password must be string");

    strcpy(config->password, StringValueCStr(option_tmp));
  }

  option_tmp = rb_hash_aref(options, user_sym);
  if ( option_tmp != Qnil ) { // user
    if ( TYPE(option_tmp) != T_STRING )
      rb_raise(rb_eRuntimeError, "[AerospikeC::Client][initialize] options :user must be string");

    strcpy(config->user, StringValueCStr(option_tmp));
  }

  option_tmp = rb_hash_aref(options, interval_sym);
  if ( option_tmp != Qnil ) { // interval
    if ( TYPE(option_tmp) != T_FIXNUM )
      rb_raise(rb_eRuntimeError, "[AerospikeC::Client][initialize] options :interval must be an integer");

    config->tender_interval = FIX2INT(option_tmp);
  }

  option_tmp = rb_hash_aref(options, thread_pool_size_sym);
  if ( option_tmp != Qnil ) { // thread_pool_size
    if ( TYPE(option_tmp) != T_FIXNUM )
      rb_raise(rb_eRuntimeError, "[AerospikeC::Client][initialize] options :thread_pool_size must be an integer");

    config->thread_pool_size = FIX2INT(option_tmp);
  }

  option_tmp = rb_hash_aref(options, max_threads_sym);
  if ( option_tmp != Qnil ) { // max_threads
    if ( TYPE(option_tmp) != T_FIXNUM )
      rb_raise(rb_eRuntimeError, "[AerospikeC::Client][initialize] options :max_threads must be an integer");

    config->max_threads = FIX2INT(option_tmp);
  }

  option_tmp = rb_hash_aref(options, conn_timeout_sym);
  if ( option_tmp != Qnil ) { // conn_timeout
    if ( TYPE(option_tmp) != T_FIXNUM )
      rb_raise(rb_eRuntimeError, "[AerospikeC::Client][initialize] options :conn_timeout must be an integer");

    config->conn_timeout_ms = FIX2INT(option_tmp);
  }

  option_tmp = rb_hash_aref(options, fail_not_connected_sym);
  if ( option_tmp != Qnil ) { // fail_not_connected
    if ( TYPE(option_tmp) != T_TRUE && TYPE(option_tmp) != T_FALSE )
      rb_raise(rb_eRuntimeError, "[AerospikeC::Client][initialize] options :fail_not_connected must be true/false");

    config->fail_if_not_connected = rb_bool2bool(option_tmp);
  }

  option_tmp = rb_hash_aref(options, logger_sym);
  if ( option_tmp != Qnil ) Logger = option_tmp; // logger
}

// ----------------------------------------------------------------------------------
//
// init scan priority with options
//
void set_priority_options(as_scan * scan, VALUE options) {
  VALUE priority = rb_hash_aref(options, priority_sym);

  if ( priority != Qnil && TYPE(priority) == T_FIXNUM ) {
    as_scan_set_priority(scan, FIX2INT(priority));
  }
}

// ----------------------------------------------------------------------------------
//
// get as_policy_query pointer from AerospikeC::Query object
//
as_policy_query * get_query_policy(VALUE query_obj) {
  as_policy_query * policy;

  VALUE rb_policy = rb_iv_get(query_obj, "@policy");

  if ( rb_policy != Qnil ) {
    Data_Get_Struct(rb_iv_get(rb_policy, "policy"), as_policy_query, policy);
    return policy;
  }
  else {
    return NULL;
  }
}