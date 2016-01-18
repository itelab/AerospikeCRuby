/*******************************************************************

  Adam Stomski, for Itelab (http://www.itelab.eu/)
  Aerospike ruby client, using aerospike-c-client

  Written and tested with:
    * aerospike          - 3.6.2
    * aerospike-c-client - 3.1.24
    * ruby version       - 2.2.2
    * Ubuntu             - 15.04

********************************************************************/

#include <aerospike_c_ruby.h>

// ----------------------------------------------------------------------------------
// Init
//
void Init_aerospike_c_ruby() {
  VALUE AerospikeC = rb_define_module("AerospikeC");

  rb_define_const(AerospikeC, "VERSION", rb_str_new2("0.2.0"));
  rb_define_const(AerospikeC, "C_CLIENT_VERSION", rb_str_new2("3.1.24"));

  init_aerospike_c_client(AerospikeC);
  init_aerospike_c_key(AerospikeC);
  init_aerospike_c_record(AerospikeC);
  init_aerospike_c_geo_json(AerospikeC);
  init_aerospike_c_operation(AerospikeC);
  init_aerospike_c_index_task(AerospikeC);
  init_aerospike_c_udf_task(AerospikeC);
  init_aerospike_c_scan_task(AerospikeC);
  init_aerospike_c_query(AerospikeC);
  init_aerospike_c_query_task(AerospikeC);
  init_aerospike_c_policy(AerospikeC);
  init_aerospike_c_llist(AerospikeC);
  init_aerospike_c_ldt_proxy(AerospikeC);
  init_aerospike_c_exceptions(AerospikeC);
}