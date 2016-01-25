#include <aerospike_c_ruby.h>

VALUE PasswordEngine;

static VALUE password_create(VALUE self, VALUE pass) {
  char salt[30];
  char password_hash[AS_PASSWORD_HASH_SIZE];

  as_password_gen_salt(salt);
  as_password_gen_hash(StringValueCStr(pass), salt, password_hash);

  return rb_str_new2(password_hash);
}

static VALUE password_gen_salt(VALUE self) {
  char salt[30];

  as_password_gen_salt(salt);

  return rb_str_new2(salt);
}

static VALUE password_hash_secret(VALUE self, VALUE pass, VALUE salt) {
  char password_hash[AS_PASSWORD_HASH_SIZE];

  as_password_gen_hash(StringValueCStr(pass), StringValueCStr(salt), password_hash);

  return rb_str_new2(password_hash);
}


// ----------------------------------------------------------------------------------
//
// Init
//
void init_aerospike_c_password(VALUE AerospikeC) {
  //
  // class AerospikeC::PasswordEngine < Object
  //
  PasswordEngine = rb_define_class_under(AerospikeC, "PasswordEngine", rb_cString);

  rb_define_singleton_method(PasswordEngine, "create", RB_FN_ANY()password_create, 1);
  rb_define_singleton_method(PasswordEngine, "gen_salt", RB_FN_ANY()password_gen_salt, 0);
  rb_define_singleton_method(PasswordEngine, "hash_secret", RB_FN_ANY()password_hash_secret, 2);
}