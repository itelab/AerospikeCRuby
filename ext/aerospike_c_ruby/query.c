#include <aerospike_c_ruby.h>

VALUE rb_aero_Query;

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
    if ( TYPE(bins) != T_ARRAY) rb_raise(rb_aero_OptionError, "[AerospikeC::Query][initialize] bins must be an array: %s", rb_val_type_as_str(bins));
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
      rb_raise(rb_aero_OptionError, "[AerospikeC::Query][eql] Unsuporrted value type: %s", rb_val_type_as_str(value));
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
  if ( TYPE(min) != T_FIXNUM ) rb_raise(rb_aero_OptionError, "[AerospikeC::Query][range] min must be integer");
  if ( TYPE(max) != T_FIXNUM ) rb_raise(rb_aero_OptionError, "[AerospikeC::Query][range] max must be integer");

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
// geo_contains!(bin, geo)
//
static VALUE geo_contains(VALUE self, VALUE bin, VALUE geo) {
  if ( TYPE(geo) != T_DATA || rb_funcall(geo, rb_intern("is_a?"), 1, rb_aero_GeoJson) != Qtrue )
    rb_raise(rb_aero_OptionError, "[AerospikeC::Query][geo_contains] must be AerospikeC::GeoJson obj");

  VALUE filter = rb_hash_new();

  rb_hash_aset(filter, filter_type_sym, geo_contains_sym);
  rb_hash_aset(filter, bin_sym, value_to_s(bin));
  rb_hash_aset(filter, value_sym, geo);

  rb_iv_set(self, "@filter", filter);

  return self;
}

// ----------------------------------------------------------------------------------
//
// geo_within!(bin, geo)
//
static VALUE geo_within(VALUE self, VALUE bin, VALUE geo) {
  if ( TYPE(geo) != T_DATA || rb_funcall(geo, rb_intern("is_a?"), 1, rb_aero_GeoJson) != Qtrue )
    rb_raise(rb_aero_OptionError, "[AerospikeC::Query][geo_within] must be AerospikeC::GeoJson obj");

  VALUE filter = rb_hash_new();

  rb_hash_aset(filter, filter_type_sym, geo_within_sym);
  rb_hash_aset(filter, bin_sym, value_to_s(bin));
  rb_hash_aset(filter, value_sym, geo);

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
    rb_raise(rb_aero_OptionError, "[AerospikeC::Query][order_by] order value: %s, should be :desc or :asc", val_inspect(order));
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
//
// bins<<(bin)
//
// params:
//   bin - bin to add
//
//  ------
//  RETURN:
//    1. bins
//
static VALUE add_bin(VALUE self, VALUE bin) {
  VALUE bins = rb_iv_get(self, "@bins");

  rb_ary_push(bins, bin);
  rb_iv_set(self, "@bins", bins);

  return bins;
}

// ----------------------------------------------------------------------------------
//
// policy=(policy)
//
// params:
//   policy - AerospikeC::QueryPolicy
//
//  ------
//  RETURN:
//    1. bins
//
static VALUE set_policy(VALUE self, VALUE policy) {
  if ( rb_funcall(policy, rb_intern("is_a?"), 1, rb_aero_QueryPolicy) == Qfalse )
    rb_raise(rb_aero_OptionError, "[AerospikeC::Query][policy=] use AerospikeC::QueryPolicy as policy: %s", val_inspect(policy));

  rb_iv_set(self, "@policy", policy);

  return self;
}

// ----------------------------------------------------------------------------------
//
// query_info
//
static VALUE query_info(VALUE self) {
  VALUE filter = rb_iv_get(self, "@filter");
  VALUE bins = rb_iv_get(self, "@bins");

  VALUE filter_type = rb_hash_aref(filter, filter_type_sym);
  VALUE query_bin   = rb_hash_aref(filter, bin_sym);

  char * select_info;

  if ( rb_ary_len_int(bins) == 0 ) {
    select_info = "*";
  }
  else {
    select_info = value_to_s_cstr(bins);
  }

  if ( filter_type == eql_sym ) {
    VALUE val        = rb_hash_aref(filter, value_sym);

    return rb_sprintf("SELECT %s WHERE %s = %s", select_info, StringValueCStr(query_bin), value_to_s_cstr(val));
  }
  else if ( filter_type == range_sym ) {
    VALUE min = rb_hash_aref(filter, min_sym);
    VALUE max = rb_hash_aref(filter, max_sym);

    return rb_sprintf("SELECT %s WHERE %s BETWEEN %s AND %s", select_info, StringValueCStr(query_bin), value_to_s_cstr(min), value_to_s_cstr(max));
  }
}


/**
 * @brief      Set @with_header
 *
 * @param[in]  self  The object.
 * @param[in]  val   value to set
 *
 * @return     self
 */
static VALUE set_with_header(VALUE self, VALUE val) {
  rb_iv_set(self, "@with_header", val);

  return self;
}

/**
 * @brief      Get the with header.
 *
 * @param[in]  self  The object.
 *
 * @return     @with_header
 */
static VALUE get_with_header(VALUE self) {
  return rb_iv_get(self, "@with_header");
}

// ----------------------------------------------------------------------------------
// Init
//
void init_aerospike_c_query(VALUE AerospikeC) {
  //
  // class AerospikeC::Query < Object
  //
  rb_aero_Query = rb_define_class_under(AerospikeC, "Query", rb_cObject);

  //
  // methods
  //
  rb_define_method(rb_aero_Query, "initialize", RB_FN_ANY()query_initialize, -1);

  rb_define_method(rb_aero_Query, "eql!", RB_FN_ANY()eql, 2);
  rb_define_method(rb_aero_Query, "range!", RB_FN_ANY()range, 3);
  rb_define_method(rb_aero_Query, "geo_contains!", RB_FN_ANY()geo_contains, 2);
  rb_define_method(rb_aero_Query, "geo_within!", RB_FN_ANY()geo_within, 2);

  rb_define_method(rb_aero_Query, "order_by!", RB_FN_ANY()order_by, 2);
  rb_define_method(rb_aero_Query, "bins<<", RB_FN_ANY()add_bin, 1);
  rb_define_method(rb_aero_Query, "policy=", RB_FN_ANY()set_policy, 1);
  rb_define_method(rb_aero_Query, "query_info", RB_FN_ANY()query_info, 0);
  rb_define_method(rb_aero_Query, "with_header!", RB_FN_ANY()set_with_header, 1);
  rb_define_method(rb_aero_Query, "with_header", RB_FN_ANY()get_with_header, 0);

  //
  // attr_accessor
  //
  rb_define_attr(rb_aero_Query, "namespace", 1, 1);
  rb_define_attr(rb_aero_Query, "set", 1, 1);
  rb_define_attr(rb_aero_Query, "bins", 1, 1);

  //
  // attr_reader
  //
  rb_define_attr(rb_aero_Query, "filter", 1, 0);
  rb_define_attr(rb_aero_Query, "order", 1, 0);
}