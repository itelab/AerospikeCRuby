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
// def initialize(value, options = {})
// @TODO options policy
//
static void rec_initialize(int argc, VALUE * argv, VALUE self) {
  VALUE value;
  VALUE options;

  rb_scan_args(argc, argv, "11", &value, &options);

  if ( NIL_P(options) ) {
    options = rb_hash_new();
    rb_hash_aset(options, ttl_sym, rb_zero);
  }
  else {
    if ( TYPE(rb_hash_aref(options, ttl_sym)) != T_FIXNUM ) { // check ttl option
      rb_raise(rb_eRuntimeError, "[AerospikeC::Client][put] ttl must be an integer");
    }
  }

  rb_iv_set(self, "@ttl", rb_hash_aref(options, ttl_sym));
  rb_iv_set(self, "@bins", value);

  long len = rb_ary_len_long(value);

  as_record * rec = (as_record *) malloc( sizeof(as_record) );
  as_record_init(rec, len);
  rec->ttl = FIX2INT( rb_hash_aref(options, ttl_sym) );

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

//
// def bins=(value)
//
static VALUE set_bins(VALUE self, VALUE value) {
  rb_iv_set(self, "@bins", value);

  long len = rb_ary_len_long(value);

  as_record * rec = (as_record *) malloc( sizeof(as_record) );
  as_record_init(rec, len);
  rec->ttl = FIX2INT( rb_funcall(self, rb_intern("ttl"), 0) );

  VALUE record = Data_Wrap_Struct(Record, NULL, rec_deallocate, rec);

  hash2record(value, record);

  rb_iv_set(self, "rec", record);
}

//
// def ttl=(seconds)
//
static VALUE set_ttl(VALUE self, VALUE ttl) {
  if ( TYPE(ttl) != T_FIXNUM ) {
    rb_raise(rb_eRuntimeError, "[AerospikeC::Client][put] ttl must be an integer");
  }

  rb_iv_set(self, "@ttl", ttl);
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
  rb_define_method(Record, "initialize", RB_FN_ANY()rec_initialize, -1);
  rb_define_method(Record, "length", RB_FN_ANY()length, 0);
  rb_define_method(Record, "bins=", RB_FN_ANY()set_bins, 1);
  rb_define_method(Record, "ttl=", RB_FN_ANY()set_ttl, 1);

  //
  // attr_reader
  //
  rb_define_attr(Record, "bins", 1, 0);
  rb_define_attr(Record, "ttl", 1, 0);
}