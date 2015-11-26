#include <aerospike_c_ruby.h>

VALUE Operation;

//
// def initialize(operations = [], options = {})
// @TODO options policy
//
static VALUE operation_initialize(int argc, VALUE * argv, VALUE self) {
  VALUE operations;
  VALUE options;

  rb_scan_args(argc, argv, "02", &operations, &options);

  // default values for optional arguments
  if ( NIL_P(options) ) {
    options = rb_hash_new();
    rb_hash_aset(options, ttl_sym, rb_zero);
  }
  else {
    if ( TYPE(rb_hash_aref(options, ttl_sym)) != T_FIXNUM ) { // check ttl option
      rb_raise(rb_eRuntimeError, "[AerospikeC::Operation][initialize] ttl must be an integer");
    }
  }

  if ( NIL_P(operations) ) {
    operations = rb_ary_new();
  }
  else {
    if ( TYPE(operations) != T_ARRAY ) {
      rb_raise(rb_eRuntimeError, "[AerospikeC::Operation][initialize] operations must be and array");
    }
  }

  rb_iv_set(self, "@operations", operations);
  rb_iv_set(self, "@ttl", rb_hash_aref(options, ttl_sym));
}

//
// def ttl=(seconds)
//
static VALUE set_ttl(VALUE self, VALUE ttl) {
  if ( TYPE(ttl) != T_FIXNUM ) {
    rb_raise(rb_eRuntimeError, "[AerospikeC::Operation][ttl=] ttl must be an integer");
  }

  rb_iv_set(self, "@ttl", ttl);

  return self;
}

//
// def <<(operation)
//
static VALUE set_operation(VALUE self, VALUE operation) {
  VALUE operations = rb_iv_get(self, "@operations");

  rb_ary_push(operations, operation);
  rb_iv_set(self, "@operations", operations);

  return self;
}

//
// def count
//
static VALUE count_operations(VALUE self) {
  return rb_funcall(rb_iv_get(self, "@operations"), rb_intern("length"), 0);
}

//
// def increment!(bin)
//
static VALUE op_increment(VALUE self, VALUE bin, VALUE val) {
  VALUE bin_name;

  if ( TYPE(bin) == T_STRING ) {
    bin_name = bin;
  }
  else {
    bin_name = value_to_s(bin);
  }

  if ( TYPE(val) != T_FIXNUM ) {
    rb_raise(rb_eRuntimeError, "[AerospikeC::Operation][increment!] value must be an integer");
  }

  VALUE operations = rb_iv_get(self, "@operations");
  VALUE op = rb_hash_new();

  rb_hash_aset(op, operation_sym, increment_sym);
  rb_hash_aset(op, bin_sym, bin_name);
  rb_hash_aset(op, value_sym, val);

  rb_ary_push(operations, op);
  rb_iv_set(self, "@operations", operations);

  return self;
}

//
// def touch!
//
static VALUE op_touch(VALUE self) {
  VALUE operations = rb_iv_get(self, "@operations");
  VALUE op = rb_hash_new();

  rb_hash_aset(op, operation_sym, touch_sym);

  rb_ary_push(operations, op);
  rb_iv_set(self, "@operations", operations);

  return self;
}

//
// def append!(bin, val)
//
static VALUE op_append(VALUE self, VALUE bin, VALUE val) {
  VALUE bin_name;
  VALUE value;

  if ( TYPE(bin) == T_STRING ) {
    bin_name = bin;
  }
  else {
    bin_name = value_to_s(bin);
  }

  if ( TYPE(val) == T_STRING ) {
    value = val;
  }
  else {
    value = value_to_s(val);
  }

  VALUE operations = rb_iv_get(self, "@operations");
  VALUE op = rb_hash_new();

  rb_hash_aset(op, operation_sym, append_sym);
  rb_hash_aset(op, bin_sym, bin_name);
  rb_hash_aset(op, value_sym, value);

  rb_ary_push(operations, op);
  rb_iv_set(self, "@operations", operations);

  return self;
}

//
// def prepend!(bin, val)
//
static VALUE op_prepend(VALUE self, VALUE bin, VALUE val) {
  VALUE bin_name;
  VALUE value;

  if ( TYPE(bin) == T_STRING ) {
    bin_name = bin;
  }
  else {
    bin_name = value_to_s(bin);
  }

  if ( TYPE(val) == T_STRING ) {
    value = val;
  }
  else {
    value = value_to_s(val);
  }

  VALUE operations = rb_iv_get(self, "@operations");
  VALUE op = rb_hash_new();

  rb_hash_aset(op, operation_sym, prepend_sym);
  rb_hash_aset(op, bin_sym, bin_name);
  rb_hash_aset(op, value_sym, value);

  rb_ary_push(operations, op);
  rb_iv_set(self, "@operations", operations);

  return self;
}

//
// def read!(bin)
//
static VALUE op_read(VALUE self, VALUE bin) {
  VALUE bin_name;

  if ( TYPE(bin) == T_STRING ) {
    bin_name = bin;
  }
  else {
    bin_name = value_to_s(bin);
  }

  VALUE operations = rb_iv_get(self, "@operations");
  VALUE op = rb_hash_new();

  rb_hash_aset(op, operation_sym, read_sym);
  rb_hash_aset(op, bin_sym, bin_name);

  rb_ary_push(operations, op);
  rb_iv_set(self, "@operations", operations);

  return self;
}

//
// def write!(bin, val)
//
static VALUE op_write(VALUE self, VALUE bin, VALUE val) {
  VALUE bin_name;

  if ( TYPE(bin) == T_STRING ) {
    bin_name = bin;
  }
  else {
    bin_name = value_to_s(bin);
  }

  VALUE operations = rb_iv_get(self, "@operations");
  VALUE op = rb_hash_new();

  rb_hash_aset(op, operation_sym, write_sym);
  rb_hash_aset(op, bin_sym, bin_name);
  rb_hash_aset(op, value_sym, val);

  rb_ary_push(operations, op);
  rb_iv_set(self, "@operations", operations);

  return self;
}

// ----------------------------------------------------------------------------------
//
// Init
//
void init_aerospike_c_operation(VALUE AerospikeC) {
  //
  // class AerospikeC::Operation < Object
  //
  Operation = rb_define_class_under(AerospikeC, "Operation", rb_cObject);

  //
  // methods
  //
  rb_define_method(Operation, "initialize", RB_FN_ANY()operation_initialize, -1);
  rb_define_method(Operation, "ttl=", RB_FN_ANY()set_ttl, 1);
  rb_define_method(Operation, "<<", RB_FN_ANY()set_operation, 1);
  rb_define_method(Operation, "count", RB_FN_ANY()count_operations, 0);
  rb_define_method(Operation, "increment!", RB_FN_ANY()op_increment, 2);
  rb_define_method(Operation, "touch!", RB_FN_ANY()op_touch, 0);
  rb_define_method(Operation, "append!", RB_FN_ANY()op_append, 2);
  rb_define_method(Operation, "prepend!", RB_FN_ANY()op_prepend, 2);
  rb_define_method(Operation, "read!", RB_FN_ANY()op_read, 1);
  rb_define_method(Operation, "write!", RB_FN_ANY()op_write, 2);

  //
  // attr_accessor
  //
  rb_define_attr(Operation, "operations", 1, 1);
  rb_define_attr(Operation, "ttl", 1, 1);
}