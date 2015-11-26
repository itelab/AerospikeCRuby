#include <aerospike_c_ruby.h>



// ----------------------------------------------------------------------------------
// Init
//
void Init_aerospike_c_ruby() {
  VALUE AerospikeC = rb_define_module("AerospikeC");

  init_aerospike_c_client(AerospikeC);
  init_aerospike_c_key(AerospikeC);
  init_aerospike_c_record(AerospikeC);
  init_aerospike_c_operation(AerospikeC);
}