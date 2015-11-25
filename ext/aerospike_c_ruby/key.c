#include <aerospike_c_ruby.h>

VALUE Key;

static char * arg_to_cstr(VALUE key);

//
// free memory method
//
static void key_deallocate(as_key * key) {
  as_key_destroy(key);
  free(key);
}

//
// def initialize
//
static void key_initialize(VALUE self, VALUE as_namespace, VALUE set, VALUE key) {
  VALUE key_struct;

  char * c_namespace = arg_to_cstr(as_namespace);
  char * c_set = arg_to_cstr(set);
  char * c_key = arg_to_cstr(key);

  as_key * k = (as_key *) malloc ( sizeof(as_key) );
  as_key_init(k, c_namespace, c_set, c_key);

  if ( k == NULL ) {
    rb_raise(rb_eRuntimeError, "Initialization key %s:%s:%s failed", c_namespace, c_set, c_key);
  }

  rb_iv_set(self, "@namespace", as_namespace);
  rb_iv_set(self, "@set", set);
  rb_iv_set(self, "@key", key);

  key_struct = Data_Wrap_Struct(Key, NULL, key_deallocate, k);

  rb_iv_set(self, "as_key", key_struct);

  log_debug("[AerospikeC::Key][initialize] initializing key");
}

//
// key to char * key_name
//
static char * arg_to_cstr(VALUE key) {
  VALUE tmp;

  switch ( TYPE(key) ) {
    case T_NIL:
      rb_raise(rb_eRuntimeError, "AerospikeC::Key][initialize] argument cannot be nil");
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
// Init
//
void init_aerospike_c_key(VALUE AerospikeC) {
  //
  // class AerospikeC::Key < Object
  //
  Key = rb_define_class_under(AerospikeC, "Key", rb_cObject);

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