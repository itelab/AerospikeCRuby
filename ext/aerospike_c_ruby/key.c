#include <aerospike_c_ruby.h>

VALUE Key;

// ----------------------------------------------------------------------------------
//
// key to char * key_name
//
static char * arg_to_cstr(VALUE key) {
  VALUE tmp;

  switch ( TYPE(key) ) {
    case T_NIL:
      rb_raise(OptionError, "[AerospikeC::Key][initialize] argument cannot be nil");
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
  xfree(key);
}

static VALUE key_allocate(VALUE self) {
  as_key * k = (as_key *) ruby_xmalloc ( sizeof(as_key) );
  if (! k) rb_raise(MemoryError, "[AerospikeC::Key][initialize] Error while allocating memory for aerospike key");

  return Data_Wrap_Struct(self, NULL, key_deallocate, k);
}

// ----------------------------------------------------------------------------------
//
// def initialize
//
static void key_initialize(VALUE self, VALUE as_namespace, VALUE set, VALUE key) {
  as_key * k;
  Data_Get_Struct(self, as_key, k);

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
  rb_funcall(str, rb_intern("<<"), 1, rb_iv_get(self, "@key"));
  rb_str_cat2(str, ">");

  return str;
}

// ----------------------------------------------------------------------------------
// Init
//
void init_aerospike_c_key(VALUE AerospikeC) {
  //
  // class AerospikeC::Key < Object
  //
  Key = rb_define_class_under(AerospikeC, "Key", rb_cObject);
  rb_define_alloc_func(Key, key_allocate);

  //
  // methods
  //
  rb_define_method(Key, "initialize", RB_FN_ANY()key_initialize, 3);
  rb_define_method(Key, "key_info", RB_FN_ANY()key_info, 0);
  rb_define_method(Key, "namespace", RB_FN_ANY()key_namespace, 0);
  rb_define_method(Key, "set", RB_FN_ANY()key_set, 0);
  rb_define_method(Key, "inspect", RB_FN_ANY()key_inspect, 0);

  //
  // attr_reader
  //
  rb_define_attr(Key, "key", 1, 0);
}