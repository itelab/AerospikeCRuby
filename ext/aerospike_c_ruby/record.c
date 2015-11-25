#include <aerospike_c_ruby.h>

VALUE Record;

//
// get bin @bins
//
static VALUE bin_val(VALUE self) {
  return rb_iv_get(self, "@bins");
}

//
// free memory method
//
static void rec_deallocate(as_record * rec) {
  as_record_destroy(rec);
  free(rec);
}

//
// def initialize
//
static void rec_initialize(VALUE self, VALUE value) {
  log_debug("[AerospikeC::Record][initialize] initializing key");

  rb_iv_set(self, "@bins", value);

  long len = rb_ary_len_long(value);

  as_record * rec = (as_record *) malloc( sizeof(as_record) );
  as_record_init(rec, len);

  VALUE record = Data_Wrap_Struct(Record, NULL, rec_deallocate, rec);

  hash2record(value, record);

  rb_iv_set(self, "rec", record);
}

//
// def length
//
static VALUE length(VALUE self) {
  return rb_funcall(bin_val(self), rb_intern("length"), 0);
}



// ----------------------------------------------------------------------------------
// Init
//
void init_aerospike_c_record(VALUE AerospikeC) {
  //
  // class AerospikeC::Record < Object
  //
  Record = rb_define_class_under(AerospikeC, "Record", rb_cObject);

  //
  // methods
  //
  rb_define_method(Record, "initialize", RB_FN_ANY()rec_initialize, 1);
  rb_define_method(Record, "length", RB_FN_ANY()length, 0);

  //
  // attr_reader
  //
  rb_define_attr(Record, "bins", 1, 0);
}