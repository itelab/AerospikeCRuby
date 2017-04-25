#include <aerospike_c_ruby.h>

VALUE rb_aero_PredExp;

// ----------------------------------------------------------------------------------
// def initialize(namespace, set, bins = [])
//
static VALUE predexp_initialize(VALUE self) {
  rb_iv_set(self, "@predexp", rb_ary_new());
}

// ----------------------------------------------------------------------------------
// def eql(bin, value)
//
static VALUE eql(VALUE self, VALUE bin, VALUE val) {
  add_numeric_or_string_predicate(self, bin, val, predexp_equal_sym);
  return self;
}

// ----------------------------------------------------------------------------------
// def uneql(bin, value)
//
static VALUE uneql(VALUE self, VALUE bin, VALUE val) {
  add_numeric_or_string_predicate(self, bin, val, predexp_unequal_sym);
  return self;
}

// ----------------------------------------------------------------------------------
// def greater(bin, value)
//
static VALUE greater(VALUE self, VALUE bin, VALUE val) {
  add_numeric_predicate(self, bin, val, predexp_greater_sym);
  return self;
}

// ----------------------------------------------------------------------------------
// def greatereq(bin, value)
//
static VALUE greatereq(VALUE self, VALUE bin, VALUE val) {
  add_numeric_predicate(self, bin, val, predexp_greatereq_sym);
  return self;
}

// ----------------------------------------------------------------------------------
// def less(bin, value)
//
static VALUE less(VALUE self, VALUE bin, VALUE val) {
  add_numeric_predicate(self, bin, val, predexp_less_sym);
  return self;
}

// ----------------------------------------------------------------------------------
// def lesseq(bin, value)
//
static VALUE lesseq(VALUE self, VALUE bin, VALUE val) {
  add_numeric_predicate(self, bin, val, predexp_lesseq_sym);
  return self;
}

// ----------------------------------------------------------------------------------
// def within(bin, value)
//
static VALUE within(VALUE self, VALUE bin, VALUE val) {
  add_geojson_predicate(self, bin, val, predexp_within_sym);
  return self;
}

// ----------------------------------------------------------------------------------
// def contains(bin, value)
//
static VALUE contains(VALUE self, VALUE bin, VALUE val) {
  add_geojson_predicate(self, bin, val, predexp_contains_sym);
  return self;
}

// ----------------------------------------------------------------------------------
// def regexp(bin, value)
//
static VALUE regexp(VALUE self, VALUE bin, VALUE val) {
  add_string_predicate(self, bin, val, predexp_regexp_sym);
  return self;
}

// ----------------------------------------------------------------------------------
// def expiration_time(predicate, value)
//
static VALUE record_expiration_time(VALUE self, VALUE pred, VALUE val) {
  add_record_predicate(self, pred, val, predexp_record_expiration_time_sym);
  return self;
}

// ----------------------------------------------------------------------------------
// def last_update(predicate, value)
//
static VALUE record_last_update(VALUE self, VALUE pred, VALUE val) {
  add_record_predicate(self, pred, val, predexp_record_last_update_sym);
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

void add_record_predicate(VALUE self, VALUE pred, VALUE val, VALUE sym){
  VALUE predicate = rb_hash_new();

  rb_hash_aset(predicate, predexp_sym, predexp_record_sym);
  rb_hash_aset(predicate, predexp_record_predexp_type_sym, sym);

  switch(TYPE(pred)) {
    case T_SYMBOL:
      if( pred == predexp_equal_sym || pred == predexp_unequal_sym || pred == predexp_greater_sym || pred == predexp_greatereq_sym || pred == predexp_less_sym || pred == predexp_lesseq_sym) {
        rb_hash_aset(predicate, predexp_record_predexp_sym, pred);
      } else {
        rb_raise(rb_aero_OptionError, "[AerospikeC::PredExp] Unsupported predicate");
      }
      break;

    default:
      rb_raise(rb_aero_OptionError, "[AerospikeC::PredExp] Unsupported predicate type: %s", rb_val_type_as_str(pred));
      break;
  }

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
  rb_define_method(rb_aero_PredExp, "expiration_time", RB_FN_ANY()record_expiration_time, 2);
  rb_define_method(rb_aero_PredExp, "last_update", RB_FN_ANY()record_last_update, 2);

  //
  // attr_accessor
  //
  rb_define_attr(rb_aero_PredExp, "predexp", 1, 1);
}
