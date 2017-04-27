#include <aerospike_c_ruby.h>

VALUE rb_aero_PredExp;

// ----------------------------------------------------------------------------------
// def initialize
//
static VALUE predexp_initialize(VALUE self) {
  rb_iv_set(self, "@predexp", Qnil);
}

// ----------------------------------------------------------------------------------
// Init
//
void init_aerospike_c_predexp(VALUE AerospikeC) {
  //
  // class AerospikeC::PredExp < Object
  //
  rb_aero_PredExp = rb_define_class_under(AerospikeC, "PredExp", rb_cObject);

  //
  // methods
  //
  rb_define_method(rb_aero_PredExp, "initialize", RB_FN_ANY()predexp_initialize, 0);

  //
  // attr_accessor
  //
  rb_define_attr(rb_aero_PredExp, "predexp", 1, 1);
}
