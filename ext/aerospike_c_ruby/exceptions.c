#include <aerospike_c_ruby.h>

VALUE rb_aero_AsError;
VALUE rb_aero_MemoryError;
VALUE rb_aero_ParseError;
VALUE rb_aero_OptionError;



// ----------------------------------------------------------------------------------
//
// Init
//
void init_aerospike_c_exceptions(VALUE AerospikeC) {
  //
  // class AerospikeC::AsError < StandardError
  //
  rb_aero_AsError = rb_define_class_under(AerospikeC, "AsError", rb_eStandardError);

  //
  // class AerospikeC::MemoryError < AsError
  //
  rb_aero_MemoryError = rb_define_class_under(AerospikeC, "MemoryError", rb_aero_AsError);

  //
  // class AerospikeC::ParseError < AsError
  //
  rb_aero_ParseError = rb_define_class_under(AerospikeC, "ParseError", rb_aero_AsError);

  //
  // class AerospikeC::OptionError < AsError
  //
  rb_aero_OptionError = rb_define_class_under(AerospikeC, "OptionError", rb_aero_AsError);
}