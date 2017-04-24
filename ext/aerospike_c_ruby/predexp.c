#include <aerospike_c_ruby.h>

VALUE rb_aero_PredExp;

// ----------------------------------------------------------------------------------
// def initialize(namespace, set, bins = [])
//
static VALUE predexp_initialize(VALUE self) {
  rb_iv_set(self, "@predexp", rb_ary_new());
}

static VALUE eql(VALUE self, VALUE bin, VALUE val) {
  add_numeric_or_string_predicate(self, bin, val, predexp_equal_sym);
  return self;
}

static VALUE uneql(VALUE self, VALUE bin, VALUE val) {
  add_numeric_or_string_predicate(self, bin, val, predexp_unequal_sym);
  return self;
}

static VALUE greater(VALUE self, VALUE bin, VALUE val) {
  add_numeric_predicate(self, bin, val, predexp_greater_sym);
  return self;
}

static VALUE greatereq(VALUE self, VALUE bin, VALUE val) {
  add_numeric_predicate(self, bin, val, predexp_greatereq_sym);
  return self;
}

static VALUE less(VALUE self, VALUE bin, VALUE val) {
  add_numeric_predicate(self, bin, val, predexp_less_sym);
  return self;
}

static VALUE lesseq(VALUE self, VALUE bin, VALUE val) {
  add_numeric_predicate(self, bin, val, predexp_lesseq_sym);
  return self;
}

static VALUE within(VALUE self, VALUE bin, VALUE val) {
  add_geojson_predicate(self, bin, val, predexp_within_sym);
  return self;
}

static VALUE contains(VALUE self, VALUE bin, VALUE val) {
  add_geojson_predicate(self, bin, val, predexp_contains_sym);
  return self;
}

static VALUE regexp(VALUE self, VALUE bin, VALUE val) {
  add_string_predicate(self, bin, val, predexp_regexp_sym);
  return self;
}


// -------------------------------------------------------------------------------------------------
// helper methods
//

void add_numeric_predicate(VALUE self, VALUE bin, VALUE val, VALUE sym) {
  VALUE predicate = rb_hash_new();

  rb_hash_aset(predicate, predexp_sym, sym);
  rb_hash_aset(predicate, bin_sym, value_to_s(bin));
  rb_hash_aset(predicate, value_sym, val);

  switch ( TYPE(val) ) {
    case T_FIXNUM:
      rb_hash_aset(predicate, type_sym, numeric_sym);
      break;

    default:
      rb_raise(rb_aero_OptionError, "[AerospikeC::PredExp] Unsupported value type: %s", rb_val_type_as_str(val));
      break;
  }

  save_predicate(self, predicate);
}

void add_numeric_or_string_predicate(VALUE self, VALUE bin, VALUE val, VALUE sym) {
  VALUE predicate = rb_hash_new();

  rb_hash_aset(predicate, predexp_sym, sym);
  rb_hash_aset(predicate, bin_sym, value_to_s(bin));
  rb_hash_aset(predicate, value_sym, val);

  switch ( TYPE(val) ) {
    case T_FIXNUM:
      rb_hash_aset(predicate, type_sym, numeric_sym);
      break;

    case T_STRING:
      rb_hash_aset(predicate, type_sym, string_sym);
      break;

    default:
      rb_raise(rb_aero_OptionError, "[AerospikeC::PredExp] Unsupported value type: %s", rb_val_type_as_str(val));
      break;
  }

  save_predicate(self, predicate);
}

void add_string_predicate(VALUE self, VALUE bin, VALUE val, VALUE sym) {
  VALUE predicate = rb_hash_new();

  rb_hash_aset(predicate, predexp_sym, sym);
  rb_hash_aset(predicate, bin_sym, value_to_s(bin));
  rb_hash_aset(predicate, value_sym, val);

  switch ( TYPE(val) ) {
    case T_STRING:
      rb_hash_aset(predicate, type_sym, string_sym);
      break;

    default:
      rb_raise(rb_aero_OptionError, "[AerospikeC::PredExp] Unsupported value type: %s", rb_val_type_as_str(val));
      break;
  }

  save_predicate(self, predicate);
}

void add_geojson_predicate(VALUE self, VALUE bin, VALUE val, VALUE sym) {
  VALUE predicate = rb_hash_new();

  rb_hash_aset(predicate, predexp_sym, sym);
  rb_hash_aset(predicate, bin_sym, value_to_s(bin));
  rb_hash_aset(predicate, value_sym, val);

  switch ( TYPE(val) ) {
    case T_DATA:
      if ( rb_funcall(val, rb_intern("is_a?"), 1, rb_aero_GeoJson) != Qtrue )
        rb_raise(rb_aero_OptionError, "[AerospikeC::PredExp] value must be AerospikeC::GeoJson obj");
      else
        rb_hash_aset(predicate, type_sym, geo_json_sym);
      break;

    default:
      rb_raise(rb_aero_OptionError, "[AerospikeC::PredExp] Unsupported value type: %s", rb_val_type_as_str(val));
      break;
  }

  save_predicate(self, predicate);
}

void save_predicate(VALUE self, VALUE predicate) {
  VALUE old_pred = rb_iv_get(self, "@predexp");
  rb_ary_push(old_pred, predicate);
  rb_iv_set(self, "@predexp", old_pred);
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
  rb_define_method(rb_aero_PredExp, "eql", RB_FN_ANY()eql, 2);
  rb_define_method(rb_aero_PredExp, "uneql", RB_FN_ANY()uneql, 2);
  rb_define_method(rb_aero_PredExp, "greater", RB_FN_ANY()greater, 2);
  rb_define_method(rb_aero_PredExp, "greatereq", RB_FN_ANY()greatereq, 2);
  rb_define_method(rb_aero_PredExp, "less", RB_FN_ANY()less, 2);
  rb_define_method(rb_aero_PredExp, "lesseq", RB_FN_ANY()lesseq, 2);
  rb_define_method(rb_aero_PredExp, "within", RB_FN_ANY()within, 2);
  rb_define_method(rb_aero_PredExp, "contains", RB_FN_ANY()contains, 2);
  rb_define_method(rb_aero_PredExp, "regexp", RB_FN_ANY()regexp, 2);

  //
  // attr_accessor
  //
  rb_define_attr(rb_aero_PredExp, "predexp", 1, 1);
}
