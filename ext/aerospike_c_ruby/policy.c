#include <aerospike_c_ruby.h>

VALUE Policy;
VALUE ReadPolicy;
VALUE WritePolicy;
VALUE RemovePolicy;
VALUE ApplyPolicy;
VALUE QueryPolicy;

// ----------------------------------------------------------------------------------
//
// options buffer
//
typedef struct options_buffer_s {
  as_policy_consistency_level   consistency_level;
  as_policy_commit_level        commit_level;
  as_policy_replica             replica;
  as_policy_gen                 gen;
  as_policy_key                 key;
  as_policy_exists              exists;
  uint32_t                      retry;
  uint32_t                      timeout;
  uint16_t                      generation;
} options_buffer;

// ----------------------------------------------------------------------------------
//
// memory allocate error info
//
static const char * err_memory_info() {
  return "[AerospikeC::Policy] Error while allocating memory for aerospike policy";
}

// ----------------------------------------------------------------------------------
//
// initialize buffer with default values
//
static void init_buffer(options_buffer * buffer) {
  buffer->commit_level      = AS_POLICY_COMMIT_LEVEL_ALL;
  buffer->consistency_level = AS_POLICY_CONSISTENCY_LEVEL_ONE;
  buffer->replica           = AS_POLICY_REPLICA_MASTER;
  buffer->gen               = AS_POLICY_GEN_IGNORE;
  buffer->key               = AS_POLICY_KEY_DIGEST;
  buffer->exists            = AS_POLICY_EXISTS_IGNORE;
  buffer->retry             = 1;
  buffer->timeout           = 1000;
  buffer->generation        = 0;
}

// ----------------------------------------------------------------------------------
//
// parse options
// set it into buffer
// set instance variables
//
static void options2buffer(VALUE self, VALUE options, options_buffer * buffer) {
  init_buffer(buffer);

  VALUE option_tmp = rb_hash_aref(options, commit_level_sym);
  if ( option_tmp != Qnil ) {
    buffer->commit_level = FIX2INT(option_tmp);
    rb_iv_set(self, "@commit_level", option_tmp);
  }

  option_tmp = rb_hash_aref(options, consistency_level_sym);
  if ( option_tmp != Qnil ) {
    buffer->consistency_level = FIX2INT(option_tmp);
    rb_iv_set(self, "@consistency_level", option_tmp);
  }

  option_tmp = rb_hash_aref(options, replica_sym);
  if ( option_tmp != Qnil ) {
    buffer->replica = FIX2INT(option_tmp);
    rb_iv_set(self, "@replica", option_tmp);
  }

  option_tmp = rb_hash_aref(options, gen_sym);
  if ( option_tmp != Qnil ) {
    buffer->gen = FIX2INT(option_tmp);
    rb_iv_set(self, "@gen", option_tmp);
  }

  option_tmp = rb_hash_aref(options, key_sym);
  if ( option_tmp != Qnil ) {
    buffer->key = FIX2INT(option_tmp);
    rb_iv_set(self, "@key", option_tmp);
  }

  option_tmp = rb_hash_aref(options, exists_sym);
  if ( option_tmp != Qnil ) {
    buffer->exists = FIX2INT(option_tmp);
    rb_iv_set(self, "@exists", option_tmp);
  }

  option_tmp = rb_hash_aref(options, retry_sym);
  if ( option_tmp != Qnil ) {
    buffer->retry = FIX2INT(option_tmp);
    rb_iv_set(self, "@retry", option_tmp);
  }

  option_tmp = rb_hash_aref(options, timeout_sym);
  if ( option_tmp != Qnil ) {
    buffer->timeout = FIX2INT(option_tmp);
    rb_iv_set(self, "@timeout", option_tmp);
  }

  option_tmp = rb_hash_aref(options, generation_sym);
  if ( option_tmp != Qnil ) {
    buffer->generation = FIX2INT(option_tmp);
    rb_iv_set(self, "@generation", option_tmp);
  }
}

// ----------------------------------------------------------------------------------
//
// AerospikeC::ReadPolicy
//
//  * init,
//  * free,
//  * ruby initialize
//
static void free_policy_read(as_policy_read * policy) {
  free(policy);
}

static void init_policy_read(VALUE self, VALUE options) {
  as_policy_read * policy = (as_policy_read *) malloc (sizeof(as_policy_read));
  if (! policy) rb_raise(MemoryError, err_memory_info());
  as_policy_read_init(policy);

  options_buffer buffer;
  options2buffer(self, options, &buffer);

  policy->consistency_level = buffer.consistency_level;
  policy->replica           = buffer.replica;
  policy->key               = buffer.key;
  policy->retry             = buffer.retry;
  policy->timeout           = buffer.timeout;

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
// AerospikeC::WritePolicy
//
//  * init,
//  * free,
//  * ruby initialize
//
static void free_policy_write(as_policy_write * policy) {
  free(policy);
}

static void init_policy_write(VALUE self, VALUE options) {
  as_policy_write * policy = (as_policy_write *) malloc (sizeof(as_policy_write));
  if (! policy) rb_raise(MemoryError, err_memory_info());
  as_policy_write_init(policy);

  options_buffer buffer;
  options2buffer(self, options, &buffer);

  policy->commit_level = buffer.commit_level;
  policy->exists       = buffer.exists;
  policy->gen          = buffer.gen;
  policy->key          = buffer.key;
  policy->retry        = buffer.retry;
  policy->timeout      = buffer.timeout;

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
// AerospikeC::RmovePolicy
//
//  * init,
//  * free,
//  * ruby initialize
//
static void free_policy_remove(as_policy_remove * policy) {
  free(policy);
}

static void init_policy_remove(VALUE self, VALUE options) {
  as_policy_remove * policy = (as_policy_remove *) malloc (sizeof(as_policy_remove));
  if (! policy) rb_raise(MemoryError, err_memory_info());
  as_policy_remove_init(policy);

  options_buffer buffer;
  options2buffer(self, options, &buffer);

  policy->commit_level = buffer.commit_level;
  policy->gen          = buffer.gen;
  policy->generation   = buffer.generation;
  policy->key          = buffer.key;
  policy->retry        = buffer.retry;
  policy->timeout      = buffer.timeout;

  VALUE policy_struct = Data_Wrap_Struct(Policy, NULL, free_policy_remove, policy);

  rb_iv_set(self, "policy", policy_struct);
}

static VALUE remove_policy_initialize(int argc, VALUE * argv, VALUE self) {
  VALUE options;

  rb_scan_args(argc, argv, "01", &options);

  VALUE super_args[2] = { remove_sym, options };

  rb_call_super(2, super_args);
}

// ----------------------------------------------------------------------------------
//
// AerospikeC::ApplyPolicy
//
//  * init,
//  * free,
//  * ruby initialize
//
static void free_policy_apply(as_policy_apply * policy) {
  free(policy);
}

static void init_policy_apply(VALUE self, VALUE options) {
  as_policy_apply * policy = (as_policy_apply *) malloc (sizeof(as_policy_apply));
  if (! policy) rb_raise(MemoryError, err_memory_info());
  as_policy_apply_init(policy);

  options_buffer buffer;
  options2buffer(self, options, &buffer);

  policy->commit_level = buffer.commit_level;
  policy->key          = buffer.key;
  policy->timeout      = buffer.timeout;

  VALUE policy_struct = Data_Wrap_Struct(Policy, NULL, free_policy_apply, policy);

  rb_iv_set(self, "policy", policy_struct);
}

static VALUE apply_policy_initialize(int argc, VALUE * argv, VALUE self) {
  VALUE options;

  rb_scan_args(argc, argv, "01", &options);

  VALUE super_args[2] = { apply_sym, options };

  rb_call_super(2, super_args);
}

// ----------------------------------------------------------------------------------
//
// AerospikeC::QueryPolicy
//
//  * init,
//  * free,
//  * ruby initialize
//
static void free_policy_query(as_policy_query * policy) {
  free(policy);
}

static void init_policy_query(VALUE self, VALUE options) {
  as_policy_query * policy = (as_policy_query *) malloc (sizeof(as_policy_query));
  if (! policy) rb_raise(MemoryError, err_memory_info());
  as_policy_query_init(policy);

  options_buffer buffer;
  options2buffer(self, options, &buffer);

  policy->timeout = buffer.timeout;

  VALUE policy_struct = Data_Wrap_Struct(Policy, NULL, free_policy_query, policy);

  rb_iv_set(self, "policy", policy_struct);
}

static VALUE query_policy_initialize(int argc, VALUE * argv, VALUE self) {
  VALUE options;

  rb_scan_args(argc, argv, "01", &options);

  VALUE super_args[2] = { query_sym, options };

  rb_call_super(2, super_args);
}

// ----------------------------------------------------------------------------------
//
// AerospikeC::Policy
//
// initialize(type, options = {})
//
// params:
//   type - symbol, policy type
//   options - policy options
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
  else if ( type == remove_sym ) {
    init_policy_remove(self, options);
  }
  else if ( type == apply_sym ) {
    init_policy_apply(self, options);
  }
  else if ( type == query_sym ) {
    init_policy_query(self, options);
  }
  else {
    rb_raise(OptionError, "[AerospikeC::Policy][initialize] unknown policy type");
  }

  rb_iv_set(self, "@type", type);
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
  rb_define_const(Policy, "AS_POLICY_COMMIT_LEVEL_ALL",         INT2FIX(AS_POLICY_COMMIT_LEVEL_ALL));
  rb_define_const(Policy, "AS_POLICY_COMMIT_LEVEL_MASTER",      INT2FIX(AS_POLICY_COMMIT_LEVEL_MASTER));

  rb_define_const(Policy, "AS_POLICY_CONSISTENCY_LEVEL_ONE",    INT2FIX(AS_POLICY_CONSISTENCY_LEVEL_ONE));
  rb_define_const(Policy, "AS_POLICY_CONSISTENCY_LEVEL_ALL",    INT2FIX(AS_POLICY_CONSISTENCY_LEVEL_ALL));

  rb_define_const(Policy, "AS_POLICY_EXISTS_IGNORE",            INT2FIX(AS_POLICY_EXISTS_IGNORE));
  rb_define_const(Policy, "AS_POLICY_EXISTS_CREATE",            INT2FIX(AS_POLICY_EXISTS_CREATE));
  rb_define_const(Policy, "AS_POLICY_EXISTS_UPDATE",            INT2FIX(AS_POLICY_EXISTS_UPDATE));
  rb_define_const(Policy, "AS_POLICY_EXISTS_REPLACE",           INT2FIX(AS_POLICY_EXISTS_REPLACE));
  rb_define_const(Policy, "AS_POLICY_EXISTS_CREATE_OR_REPLACE", INT2FIX(AS_POLICY_EXISTS_CREATE_OR_REPLACE));

  rb_define_const(Policy, "AS_POLICY_GEN_IGNORE",               INT2FIX(AS_POLICY_GEN_IGNORE));
  rb_define_const(Policy, "AS_POLICY_GEN_EQ",                   INT2FIX(AS_POLICY_GEN_EQ));
  rb_define_const(Policy, "AS_POLICY_GEN_GT",                   INT2FIX(AS_POLICY_GEN_GT));

  rb_define_const(Policy, "AS_POLICY_KEY_DIGEST",               INT2FIX(AS_POLICY_KEY_DIGEST));
  rb_define_const(Policy, "AS_POLICY_KEY_SEND",                 INT2FIX(AS_POLICY_KEY_SEND));

  rb_define_const(Policy, "AS_POLICY_REPLICA_MASTER",           INT2FIX(AS_POLICY_REPLICA_MASTER));
  rb_define_const(Policy, "AS_POLICY_REPLICA_ANY",              INT2FIX(AS_POLICY_REPLICA_ANY));

  rb_define_const(Policy, "AS_POLICY_RETRY_NONE",               INT2FIX(AS_POLICY_RETRY_NONE));
  rb_define_const(Policy, "AS_POLICY_RETRY_ONCE",               INT2FIX(AS_POLICY_RETRY_ONCE));

  //
  // methods
  //
  rb_define_method(Policy, "initialize", RB_FN_ANY()policy_initialize, -1);

  //
  // attr_reader
  //
  rb_define_attr(Policy, "type",              1, 0);
  rb_define_attr(Policy, "timeout",           1, 0);
  rb_define_attr(Policy, "retry",             1, 0);
  rb_define_attr(Policy, "replica",           1, 0);
  rb_define_attr(Policy, "consistency_level", 1, 0);
  rb_define_attr(Policy, "key",               1, 0);
  rb_define_attr(Policy, "exists",            1, 0);
  rb_define_attr(Policy, "gen",               1, 0);
  rb_define_attr(Policy, "commit_level",      1, 0);
  rb_define_attr(Policy, "generation",        1, 0);


  //
  // specific classes
  //
  WritePolicy = rb_define_class_under(AerospikeC, "WritePolicy", Policy);
  rb_define_method(WritePolicy, "initialize", RB_FN_ANY()write_policy_initialize, -1);

  ReadPolicy = rb_define_class_under(AerospikeC, "ReadPolicy", Policy);
  rb_define_method(ReadPolicy, "initialize", RB_FN_ANY()read_policy_initialize, -1);

  RemovePolicy = rb_define_class_under(AerospikeC, "RemovePolicy", Policy);
  rb_define_method(RemovePolicy, "initialize", RB_FN_ANY()remove_policy_initialize, -1);

  ApplyPolicy = rb_define_class_under(AerospikeC, "ApplyPolicy", Policy);
  rb_define_method(ApplyPolicy, "initialize", RB_FN_ANY()apply_policy_initialize, -1);

  QueryPolicy = rb_define_class_under(AerospikeC, "QueryPolicy", Policy);
  rb_define_method(QueryPolicy, "initialize", RB_FN_ANY()query_policy_initialize, -1);
}