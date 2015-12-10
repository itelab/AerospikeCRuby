#include <aerospike_c_ruby.h>

VALUE Policy;


// ----------------------------------------------------------------------------------
//
// read policy
//  * init,
//  * free,
//  * ruby initialize
//
static void free_policy_read(as_policy_read * policy) {
  free(policy);
}

static void init_policy_read(VALUE self, VALUE options) {
  as_policy_read * policy = (as_policy_read *) malloc (sizeof(as_policy_read));
  as_policy_read_init(policy);

  VALUE option_tmp;

  option_tmp = rb_hash_aref(options, replica_sym);
  if ( option_tmp != Qnil ) policy->replica = FIX2INT(option_tmp);

  option_tmp = rb_hash_aref(options, consistency_level_sym);
  if ( option_tmp != Qnil ) policy->consistency_level = FIX2INT(option_tmp);

  option_tmp = rb_hash_aref(options, key_sym);
  if ( option_tmp != Qnil ) policy->key = FIX2INT(option_tmp);

  option_tmp = rb_hash_aref(options, retry_sym);
  if ( option_tmp != Qnil ) policy->retry = FIX2INT(option_tmp);

  option_tmp = rb_hash_aref(options, timeout_sym);
  if ( option_tmp != Qnil ) policy->timeout = FIX2INT(option_tmp);

  VALUE policy_struct = Data_Wrap_Struct(Policy, NULL, free_policy_read, policy);

  rb_iv_set(self, "policy", policy_struct);
}

static VALUE read_policy_initialize(int argc, VALUE * argv, VALUE self) {
  VALUE options;

  rb_scan_args(argc, argv, "01", &options);

  VALUE super_args[2] = { read_sym, options };

  rb_call_super(2, super_args);
}

// ----------------------------------------------------------------------------------
//
// write policy
//  * init,
//  * free,
//  * ruby initialize
//
static void free_policy_write(as_policy_write * policy) {
  free(policy);
}

static void init_policy_write(VALUE self, VALUE options) {
  as_policy_write * policy = (as_policy_write *) malloc (sizeof(as_policy_write));
  as_policy_write_init(policy);

  VALUE option_tmp;

  option_tmp = rb_hash_aref(options, commit_level_sym);
  if ( option_tmp != Qnil ) policy->commit_level = FIX2INT(option_tmp);

  option_tmp = rb_hash_aref(options, exists_sym);
  if ( option_tmp != Qnil ) policy->exists = FIX2INT(option_tmp);

  option_tmp = rb_hash_aref(options, gen_sym);
  if ( option_tmp != Qnil ) policy->gen = FIX2INT(option_tmp);

  option_tmp = rb_hash_aref(options, key_sym);
  if ( option_tmp != Qnil ) policy->key = FIX2INT(option_tmp);

  option_tmp = rb_hash_aref(options, retry_sym);
  if ( option_tmp != Qnil ) policy->retry = FIX2INT(option_tmp);

  option_tmp = rb_hash_aref(options, timeout_sym);
  if ( option_tmp != Qnil ) policy->timeout = FIX2INT(option_tmp);

  VALUE policy_struct = Data_Wrap_Struct(Policy, NULL, free_policy_write, policy);

  rb_iv_set(self, "policy", policy_struct);
}

static VALUE write_policy_initialize(int argc, VALUE * argv, VALUE self) {
  VALUE options;

  rb_scan_args(argc, argv, "01", &options);

  VALUE super_args[2] = { write_sym, options };

  rb_call_super(2, super_args);
}

// ----------------------------------------------------------------------------------
//
// initialize(type, options = {})
//
static VALUE policy_initialize(int argc, VALUE * argv, VALUE self) {
  VALUE type;
  VALUE options;

  rb_scan_args(argc, argv, "11", &type, &options);

  if ( NIL_P(options) ) options = rb_hash_new();

  if ( type == write_sym ) {
    init_policy_write(self, options);
  }
  else if ( type == read_sym ) {
    init_policy_read(self, options);
  }
  else {
    rb_raise(rb_eRuntimeError, "[AerospikeC::Policy][initialize] unknown policy type");
  }
}

// ----------------------------------------------------------------------------------
//
// Init
//
void init_aerospike_c_policy(VALUE AerospikeC) {
  //
  // class AerospikeC::Policy < Object
  //
  Policy = rb_define_class_under(AerospikeC, "Policy", rb_cObject);

  //
  // consts
  //
  rb_define_const(Policy, "AS_POLICY_COMMIT_LEVEL_ALL", INT2FIX(AS_POLICY_COMMIT_LEVEL_ALL));
  rb_define_const(Policy, "AS_POLICY_COMMIT_LEVEL_MASTER", INT2FIX(AS_POLICY_COMMIT_LEVEL_MASTER));

  rb_define_const(Policy, "AS_POLICY_CONSISTENCY_LEVEL_ONE", INT2FIX(AS_POLICY_CONSISTENCY_LEVEL_ONE));
  rb_define_const(Policy, "AS_POLICY_CONSISTENCY_LEVEL_ALL", INT2FIX(AS_POLICY_CONSISTENCY_LEVEL_ALL));

  rb_define_const(Policy, "AS_POLICY_EXISTS_IGNORE", INT2FIX(AS_POLICY_EXISTS_IGNORE));
  rb_define_const(Policy, "AS_POLICY_EXISTS_CREATE", INT2FIX(AS_POLICY_EXISTS_CREATE));
  rb_define_const(Policy, "AS_POLICY_EXISTS_UPDATE", INT2FIX(AS_POLICY_EXISTS_UPDATE));
  rb_define_const(Policy, "AS_POLICY_EXISTS_REPLACE", INT2FIX(AS_POLICY_EXISTS_REPLACE));
  rb_define_const(Policy, "AS_POLICY_EXISTS_CREATE_OR_REPLACE", INT2FIX(AS_POLICY_EXISTS_CREATE_OR_REPLACE));

  rb_define_const(Policy, "AS_POLICY_GEN_IGNORE", INT2FIX(AS_POLICY_GEN_IGNORE));
  rb_define_const(Policy, "AS_POLICY_GEN_EQ", INT2FIX(AS_POLICY_GEN_EQ));
  rb_define_const(Policy, "AS_POLICY_GEN_GT", INT2FIX(AS_POLICY_GEN_GT));

  rb_define_const(Policy, "AS_POLICY_KEY_DIGEST", INT2FIX(AS_POLICY_KEY_DIGEST));
  rb_define_const(Policy, "AS_POLICY_KEY_SEND", INT2FIX(AS_POLICY_KEY_SEND));

  rb_define_const(Policy, "AS_POLICY_REPLICA_MASTER", INT2FIX(AS_POLICY_REPLICA_MASTER));
  rb_define_const(Policy, "AS_POLICY_REPLICA_ANY", INT2FIX(AS_POLICY_REPLICA_ANY));

  rb_define_const(Policy, "AS_POLICY_RETRY_NONE", INT2FIX(AS_POLICY_RETRY_NONE));
  rb_define_const(Policy, "AS_POLICY_RETRY_ONCE", INT2FIX(AS_POLICY_RETRY_ONCE));

  //
  // methods
  //
  rb_define_method(Policy, "initialize", RB_FN_ANY()policy_initialize, -1);


  //
  // specific classes
  //
  VALUE WritePolicy = rb_define_class_under(AerospikeC, "WritePolicy", Policy);
  rb_define_method(WritePolicy, "initialize", RB_FN_ANY()write_policy_initialize, -1);

  VALUE ReadPolicy = rb_define_class_under(AerospikeC, "ReadPolicy", Policy);
  rb_define_method(ReadPolicy, "initialize", RB_FN_ANY()read_policy_initialize, -1);
}