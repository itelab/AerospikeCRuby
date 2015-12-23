#include <aerospike_c_ruby.h>

VALUE Key;

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

//
// def initialize
//
static void key_initialize(VALUE self, VALUE as_namespace, VALUE set, VALUE key) {
  char * c_namespace = arg_to_cstr(as_namespace);
  char * c_set = arg_to_cstr(set);
  char * c_key = arg_to_cstr(key);

  as_key * k;
  Data_Get_Struct(self, as_key, k);

  as_key_init(k, c_namespace, c_set, c_key);

  if ( k == NULL ) {
    rb_raise(MemoryError, "Initialization key %s:%s:%s failed", c_namespace, c_set, c_key);
  }

  rb_iv_set(self, "@namespace", as_namespace);
  rb_iv_set(self, "@set", set);
  rb_iv_set(self, "@key", key);

  log_debug("[AerospikeC::Key][initialize] initializing key");
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

  //
  // attr_reader
  //
  rb_define_attr(Key, "namespace", 1, 0);
  rb_define_attr(Key, "set", 1, 0);
  rb_define_attr(Key, "key", 1, 0);
}