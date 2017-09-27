/*******************************************************************

  Adam Stomski, for Itelab (http://www.itelab.eu/)
  Aerospike ruby client, using aerospike-c-client

  Written and tested with:
    * aerospike          - 3.9.1.1
    * aerospike-c-client - 4.0.0
    * ruby version       - 2.2.3
    * Ubuntu             - 15.04

********************************************************************/

#include <aerospike_c_ruby.h>

VALUE rb_aero_AerospikeC;

static VALUE get_encoding(VALUE self) {
  return rb_iv_get(rb_aero_AerospikeC, "@encoding");
}

static VALUE set_encoding(VALUE self, VALUE encoding) {
  return rb_iv_set(rb_aero_AerospikeC, "@encoding", encoding);
}

static VALUE get_logger(VALUE self) {
  return rb_iv_get(rb_aero_AerospikeC, "@logger");
}

static VALUE set_logger(VALUE self, VALUE logger) {
  return rb_iv_set(rb_aero_AerospikeC, "@logger", logger);
}

static VALUE get_ttl_name(VALUE self) {
  return rb_iv_get(rb_aero_AerospikeC, "@ttl_name");
}

static VALUE set_ttl_name(VALUE self, VALUE ttl_name) {
  return rb_iv_set(rb_aero_AerospikeC, "@ttl_name", ttl_name);
}

// ----------------------------------------------------------------------------------
// Init
//
void Init_aerospike_c_ruby() {
  rb_aero_AerospikeC = rb_define_module("AerospikeC");

  rb_define_const(rb_aero_AerospikeC, "VERSION", rb_str_new2("0.5.1"));
  rb_define_const(rb_aero_AerospikeC, "C_CLIENT_VERSION", rb_str_new2("4.1.6"));

  rb_iv_set(rb_aero_AerospikeC, "@encoding", Qnil);
  rb_iv_set(rb_aero_AerospikeC, "@ttl_name", rb_str_new2("expire_in"));

  rb_define_singleton_method(rb_aero_AerospikeC, "encoding", get_encoding, 0);
  rb_define_singleton_method(rb_aero_AerospikeC, "encoding=", set_encoding, 1);
  rb_define_singleton_method(rb_aero_AerospikeC, "logger", get_logger, 0);
  rb_define_singleton_method(rb_aero_AerospikeC, "logger=", set_logger, 1);
  rb_define_singleton_method(rb_aero_AerospikeC, "ttl_name", get_ttl_name, 0);
  rb_define_singleton_method(rb_aero_AerospikeC, "ttl_name=", set_ttl_name, 1);


  init_aerospike_c_client(rb_aero_AerospikeC);
  init_aerospike_c_key(rb_aero_AerospikeC);
  init_aerospike_c_record(rb_aero_AerospikeC);
  init_aerospike_c_geo_json(rb_aero_AerospikeC);
  init_aerospike_c_index_task(rb_aero_AerospikeC);
  init_aerospike_c_udf_task(rb_aero_AerospikeC);
  init_aerospike_c_scan_task(rb_aero_AerospikeC);
  init_aerospike_c_query(rb_aero_AerospikeC);
  init_aerospike_c_query_task(rb_aero_AerospikeC);
  init_aerospike_c_policy(rb_aero_AerospikeC);
  init_aerospike_c_exceptions(rb_aero_AerospikeC);
  init_aerospike_c_password(rb_aero_AerospikeC);
  init_aerospike_c_predexp(rb_aero_AerospikeC);

  // @deprecated Starting from version 4.1.6 C Client does not support LDT
  // init_aerospike_c_llist(rb_aero_AerospikeC);
  // init_aerospike_c_ldt_proxy(rb_aero_AerospikeC);
}
