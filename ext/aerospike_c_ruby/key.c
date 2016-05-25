#include <aerospike_c_ruby.h>

VALUE rb_aero_Key;

// ----------------------------------------------------------------------------------
//
// key to char * key_name
//
static char * arg_to_cstr(VALUE key) {
  VALUE tmp;

  switch ( TYPE(key) ) {
    case T_NIL:
      rb_raise(rb_aero_OptionError, "[AerospikeC::Key][initialize] argument cannot be nil");
      break;

    case T_STRING:
      return StringValueCStr(key);
      break;

    default:
      tmp = rb_funcall(key, rb_intern("to_s"), 0);
      return StringValueCStr(tmp);
      break;
  }
}

// ----------------------------------------------------------------------------------
//
// free memory method
//
static void key_deallocate(as_key * key) {
  as_key_destroy(key);
}

static VALUE key_allocate(VALUE self) {
  as_key * k = (as_key *) malloc ( sizeof(as_key) );
  if (! k) rb_raise(rb_aero_MemoryError, "[AerospikeC::Key][initialize] Error while allocating memory for aerospike key");

  k->_free = true;

  return Data_Wrap_Struct(self, NULL, key_deallocate, k);
}

// ----------------------------------------------------------------------------------
//
// def initialize
//
static void key_initialize(VALUE self, VALUE as_namespace, VALUE set, VALUE key) {
  as_key * k;
  Data_Get_Struct(self, as_key, k);

  if ( NIL_P(as_namespace) ) {
    rb_raise(rb_aero_OptionError, "[AerospikeC::Key][initialize] namespace cannot be nil");
  }

  if ( NIL_P(set) ) {
    rb_raise(rb_aero_OptionError, "[AerospikeC::Key][initialize] set cannot be nil");
  }

  if ( NIL_P(key) ) {
    rb_raise(rb_aero_OptionError, "[AerospikeC::Key][initialize] key cannot be nil");
  }

  char * c_namespace = arg_to_cstr(as_namespace);
  char * c_set = arg_to_cstr(set);

  if ( TYPE(key) != T_FIXNUM ) {
    char * c_key = arg_to_cstr(key);
    as_key_init(k, c_namespace, c_set, c_key);
  }
  else {
    as_key_init_int64(k, c_namespace, c_set, FIX2LONG(key));
  }

  rb_iv_set(self, "@key", key);
}

// ----------------------------------------------------------------------------------
//
// def key_info
//
static VALUE key_info(VALUE self) {
  as_key * k;
  Data_Get_Struct(self, as_key, k);

  VALUE ns  = rb_str_new2(k->ns);
  VALUE set = rb_str_new2(k->set);
  VALUE key = rb_iv_get(self, "@key");

  return rb_sprintf("%"PRIsVALUE":%"PRIsVALUE":%"PRIsVALUE"", ns, set, key);
}

// ----------------------------------------------------------------------------------
//
// def namespace
//
static VALUE key_namespace(VALUE self) {
  as_key * k;
  Data_Get_Struct(self, as_key, k);

  return rb_str_new2(k->ns);
}

// ----------------------------------------------------------------------------------
//
// def set
//
static VALUE key_set(VALUE self) {
  as_key * k;
  Data_Get_Struct(self, as_key, k);

  return rb_str_new2(k->set);
}

// ----------------------------------------------------------------------------------
//
// def inspect
//
static VALUE key_inspect(VALUE self) {
  as_key * k;
  Data_Get_Struct(self, as_key, k);

  VALUE str = rb_str_new2("#<AerospikeC::Key ");
  rb_str_cat2(str, k->ns);
  rb_str_cat2(str, ":");
  rb_str_cat2(str, k->set);
  rb_str_cat2(str, ":");
  rb_funcall(str, rb_intern("<<"), 1, value_to_s(rb_iv_get(self, "@key")));
  rb_str_cat2(str, ">");

  return str;
}


/**
 * @brief      Get key digest
 *
 * @param[in]  self  The object
 *
 * @return     string digest
 */
static VALUE key_digest(VALUE self) {
  VALUE cache = rb_iv_get(self, "@digest");

  if ( TYPE(cache) == T_ARRAY ) {
    return cache;
  }

  as_key * k;
  Data_Get_Struct(self, as_key, k);

  as_digest * digest = as_key_digest(k);

  VALUE result = rb_ary_new();

  for (int i = 0; i < AS_DIGEST_VALUE_SIZE; ++i) {
    rb_ary_push(result, INT2FIX(digest->value[i]));
  }

  rb_iv_set(self, "@digest", result);

  return result;
}


/**
 * @brief      compare keys
 *
 * @param[in]  self       The object
 * @param[in]  other_key  The other key
 *
 * @return     true/false
 */
static VALUE key_eql(VALUE self, VALUE other_key) {
  if ( rb_funcall(other_key, rb_intern("is_a?"), 1, rb_aero_Key) == Qfalse ) {
    return Qfalse;
  }

  if ( rb_funcall(key_digest(self), rb_intern("=="), 1, key_digest(other_key)) == Qfalse ) {
    return Qfalse;
  }

  return Qtrue;
}


/**
 * @brief      key hash method
 *
 * @param[in]  self  The object
 *
 * @return     string digest
 */
static VALUE key_hash(VALUE self) {
  return rb_funcall(key_digest(self), rb_intern("hash"), 0);
}


// ----------------------------------------------------------------------------------
// Init
//
void init_aerospike_c_key(VALUE AerospikeC) {
  //
  // class AerospikeC::Key < Object
  //
  rb_aero_Key = rb_define_class_under(AerospikeC, "Key", rb_cObject);
  rb_define_alloc_func(rb_aero_Key, key_allocate);

  //
  // methods
  //
  rb_define_method(rb_aero_Key, "initialize", RB_FN_ANY()key_initialize, 3);
  rb_define_method(rb_aero_Key, "key_info", RB_FN_ANY()key_info, 0);
  rb_define_method(rb_aero_Key, "namespace", RB_FN_ANY()key_namespace, 0);
  rb_define_method(rb_aero_Key, "set", RB_FN_ANY()key_set, 0);
  rb_define_method(rb_aero_Key, "inspect", RB_FN_ANY()key_inspect, 0);
  rb_define_method(rb_aero_Key, "==", RB_FN_ANY()key_eql, 1);
  rb_define_method(rb_aero_Key, "digest", RB_FN_ANY()key_digest, 0);
  rb_define_method(rb_aero_Key, "hash", RB_FN_ANY()key_hash, 0);


  rb_define_alias(rb_aero_Key, "eql?", "==");

  //
  // attr_reader
  //
  rb_define_attr(rb_aero_Key, "key", 1, 0);
}