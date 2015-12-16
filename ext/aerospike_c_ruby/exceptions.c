#include <aerospike_c_ruby.h>

VALUE AsError;
VALUE MemoryError;
VALUE ParseError;
VALUE OptionError;

// ----------------------------------------------------------------------------------
//
// Init
//
void init_aerospike_c_exceptions(AerospikeC) {
  //
  // class AerospikeC::AsError < StandardError
  //
  AsError = rb_define_class_under(AerospikeC, "AsError", rb_eStandardError);

  //
  // class AerospikeC::MemoryError < AsError
  //
  MemoryError = rb_define_class_under(AerospikeC, "MemoryError", AsError);

  //
  // class AerospikeC::ParseError < AsError
  //
  ParseError = rb_define_class_under(AerospikeC, "ParseError", AsError);

  //
  // class AerospikeC::OptionError < AsError
  //
  OptionError = rb_define_class_under(AerospikeC, "OptionError", AsError);
}