#include <aerospike_c_ruby.h>

VALUE Query;

//
// def initialize(namespace, set, bins = [])
// @TODO options policy
//
static VALUE query_initialize(int argc, VALUE * argv, VALUE self) {
  VALUE ns;
  VALUE set;
  VALUE bins;

  rb_scan_args(argc, argv, "21", &ns, &set, &bins);

  if ( NIL_P(bins) ) {
    bins = rb_ary_new();
  }
  else {
    if ( TYPE(bins) != T_ARRAY) rb_raise(rb_eRuntimeError, "[AerospikeC::Query][initialize] bins must be an array: %s", rb_val_type_as_str(bins));
  }

  rb_iv_set(self, "@namespace", value_to_s(ns));
  rb_iv_set(self, "@set", value_to_s(set));
  rb_iv_set(self, "@bins", bins);
  rb_iv_set(self, "@order", rb_ary_new());
}

// ----------------------------------------------------------------------------------
//
// eql!(bin, value)
//
// params:
//   bin - bin to check equaliy
//   value - value of bin
//
//  ------
//  RETURN:
//    1. self (can chain methods)
//
static VALUE eql(VALUE self, VALUE bin, VALUE value) {
  VALUE filter = rb_hash_new();

  rb_hash_aset(filter, filter_type_sym, eql_sym);
  rb_hash_aset(filter, bin_sym, value_to_s(bin));
  rb_hash_aset(filter, value_sym, value);

  switch ( TYPE(value) ) {
    case T_STRING:
      rb_hash_aset(filter, type_sym, string_sym);
      break;

    case T_FIXNUM:
      rb_hash_aset(filter, type_sym, numeric_sym);
      break;

    default:
      rb_raise(rb_eRuntimeError, "[AerospikeC::Query][eql] Unsuporrted value type: %s", rb_val_type_as_str(value));
      break;
  }

  rb_iv_set(self, "@filter", filter);

  return self;
}

// ----------------------------------------------------------------------------------
//
// range!(bin, min, max)
//
// params:
//   bin - bin to check range
//   min - minimum value of bin
//   max - maximum value of bin
//
//  ------
//  RETURN:
//    1. self (can chain methods)
//
static VALUE range(VALUE self, VALUE bin, VALUE min, VALUE max) {
  if ( TYPE(min) != T_FIXNUM ) rb_raise(rb_eRuntimeError, "[AerospikeC::Query][range] min must be integer");
  if ( TYPE(max) != T_FIXNUM ) rb_raise(rb_eRuntimeError, "[AerospikeC::Query][range] max must be integer");

  VALUE filter = rb_hash_new();

  rb_hash_aset(filter, filter_type_sym, range_sym);
  rb_hash_aset(filter, bin_sym, value_to_s(bin));
  rb_hash_aset(filter, min_sym, min);
  rb_hash_aset(filter, max_sym, max);

  rb_iv_set(self, "@filter", filter);

  return self;
}

// ----------------------------------------------------------------------------------
//
// order_by!(bin, type)
//
// params:
//   bin - bin to order by
//   type - symbol, order type, :asc or :desc
//
//  ------
//  RETURN:
//    1. self (can chain methods)
//
static VALUE order_by(VALUE self, VALUE bin, VALUE order) {
  if ( order != asc_sym && order != desc_sym ) {
    rb_raise(rb_eRuntimeError, "[AerospikeC::Query][order_by] order value: %s, should be :desc or :asc", val_inspect(order));
  }

  VALUE q_order = rb_iv_get(self, "@order");

  VALUE filter = rb_hash_new();

  if ( order == asc_sym ) rb_hash_aset(filter, order_sym, INT2FIX(AS_ORDER_ASCENDING));
  if ( order == desc_sym ) rb_hash_aset(filter, order_sym, INT2FIX(AS_ORDER_DESCENDING));

  rb_hash_aset(filter, bin_sym, value_to_s(bin));

  rb_ary_push(q_order, filter);
  rb_iv_set(self, "@order", q_order);

  return self;
}

// ----------------------------------------------------------------------------------
// Init
//
void init_aerospike_c_query(VALUE AerospikeC) {
  //
  // class AerospikeC::Query < Object
  //
  Query = rb_define_class_under(AerospikeC, "Query", rb_cObject);

  //
  // methods
  //
  rb_define_method(Query, "initialize", RB_FN_ANY()query_initialize, -1);
  rb_define_method(Query, "eql!", RB_FN_ANY()eql, 2);
  rb_define_method(Query, "range!", RB_FN_ANY()range, 3);
  rb_define_method(Query, "order_by!", RB_FN_ANY()order_by, 2);

  //
  // attr_accessor
  //
  rb_define_attr(Query, "namespace", 1, 1);
  rb_define_attr(Query, "set", 1, 1);
  rb_define_attr(Query, "bins", 1, 1);

  //
  // attr_reader
  //
  rb_define_attr(Query, "filter", 1, 0);
  rb_define_attr(Query, "order", 1, 0);
}