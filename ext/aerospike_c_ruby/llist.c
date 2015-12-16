#include <aerospike_c_ruby.h>

VALUE Llist;

// ----------------------------------------------------------------------------------
//
// free llist
//
static void llist_free(as_ldt * llist) {
  as_ldt_destroy(llist);
  xfree(llist);
}

// ----------------------------------------------------------------------------------
//
// unwrap LList client VALUE struct into aerospike *
//
static aerospike * llist_client_struct(VALUE self) {
  return get_client_struct(rb_iv_get(self, "@client"));
}

// ----------------------------------------------------------------------------------
//
// unwrap Llist key VALUE struct into as_key *
//
static aerospike * llist_key_struct(VALUE self) {
  return get_key_struct(rb_iv_get(self, "@key"));
}

// ----------------------------------------------------------------------------------
//
// unwrap Llist VALUE struct into as_ldt *
//
static as_ldt * get_ldt_struct(VALUE self) {
  as_ldt * ldt;
  Data_Get_Struct(rb_iv_get(self, "as_ldt"), as_ldt, ldt);
  return ldt;
}

// ----------------------------------------------------------------------------------
//
// workaround for: https://discuss.aerospike.com/t/parsing-record-with-ldt/2264/2
//
static void add_llist_status_bins_workaround(VALUE client, VALUE key, VALUE bin_name) {
  if ( rb_iv_get(client, "@ldt_proxy") != Qtrue ) return;

  VALUE llist_status_ary      = rb_ary_new();
  VALUE llist_status_bin_name = RB_LLIST_WORAROUND_BIN;

  rb_ary_push(llist_status_ary, llist_status_bin_name);

  VALUE rblliststatus = rb_funcall(client, rb_intern("get"), 2, key, llist_status_ary);

  VALUE status = rb_hash_aref(rblliststatus, llist_status_bin_name);

  if ( status == Qnil ) {
    llist_status_ary = rb_ary_new();
    rb_ary_push(llist_status_ary, bin_name);
    rb_hash_aset(rblliststatus, llist_status_bin_name, llist_status_ary);

    rb_funcall(client, rb_intern("put"), 2, key, rblliststatus);
  }
  else {
    if ( rb_funcall(status, rb_intern("include?"), 1, bin_name) == Qfalse ) {
      rb_ary_push(status, bin_name);
      rb_hash_aset(rblliststatus, llist_status_bin_name, status);
      rb_funcall(client, rb_intern("put"), 2, key, rblliststatus);
    }
  }
}

// ----------------------------------------------------------------------------------
//
// def initialize(client, key, bin_name, options = {})
//
static void llist_initialize(int argc, VALUE * argv, VALUE self) {
  VALUE client;
  VALUE key;
  VALUE bin_name;
  VALUE options;

  rb_scan_args(argc, argv, "31", &client, &key, &bin_name, &options);

  if ( NIL_P(options) ) options = rb_hash_new(); // default options

  as_ldt * llist = (as_ldt *) malloc ( sizeof(as_ldt) );

  if (! llist)
    rb_raise(rb_eRuntimeError, "[AerospikeC::Llist][initialize] Error while allocating memory for aerospike llist");

  char * conf_module;

  VALUE module_name = rb_hash_aref(options, module_sym);
  if ( module_name != Qnil ) {
    conf_module = StringValueCStr(module_name);
  }
  else {
    conf_module = NULL;
  }

  if (! as_ldt_init(llist, StringValueCStr(bin_name), AS_LDT_LLIST, conf_module)) {
    llist_free(llist);
    rb_raise(rb_eRuntimeError, "[AerospikeC::Llist][initialize] Unable to initialize llist");
  }

  VALUE llist_struct = Data_Wrap_Struct(Llist, NULL, llist_free, llist);

  rb_iv_set(self, "as_ldt", llist_struct);
  rb_iv_set(self, "@bin_name", bin_name);
  rb_iv_set(self, "@client", client);
  rb_iv_set(self, "@key", key);
}

// ----------------------------------------------------------------------------------
//
// def add(value, options = {})
//
static VALUE llist_add(int argc, VALUE * argv, VALUE self) {
  as_error err;
  as_status status;
  aerospike * as = llist_client_struct(self);
  as_key * key   = llist_key_struct(self);

  VALUE value;
  VALUE options;

  rb_scan_args(argc, argv, "11", &value, &options);

  if ( NIL_P(options) ) options = rb_hash_new(); // default options

  as_policy_apply * policy = get_policy(options);
  as_val * val             = rb_val2as_val(value);
  as_ldt * llist           = get_ldt_struct(self);

  if ( ( status = aerospike_llist_add(as, &err, policy, key, llist, val) ) != AEROSPIKE_OK ) {
    as_val_free(val);

    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[AerospikeC::LList][add] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    raise_as_error(err);
  }

  as_val_free(val);

  add_llist_status_bins_workaround(rb_iv_get(self, "@client"), rb_iv_get(self, "@key"), rb_iv_get(self, "@bin_name"));

  return Qtrue;
}

// ----------------------------------------------------------------------------------
//
// def add_all(values, options = {})
//
static VALUE llist_add_all(int argc, VALUE * argv, VALUE self) {
  as_error err;
  as_status status;
  aerospike * as = llist_client_struct(self);
  as_key * key   = llist_key_struct(self);

  VALUE values;
  VALUE options;

  rb_scan_args(argc, argv, "11", &values, &options);

  if ( NIL_P(options) ) options = rb_hash_new(); // default options

  as_policy_apply * policy = get_policy(options);
  as_ldt * llist           = get_ldt_struct(self);

  as_arraylist * vals = array2as_list(values);

  if ( ( status = aerospike_llist_add_all(as, &err, policy, key, llist, (as_list *)vals) ) != AEROSPIKE_OK ) {
    as_arraylist_destroy(vals);

    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[AerospikeC::LList][add_all] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    raise_as_error(err);
  }

  as_arraylist_destroy(vals);

  add_llist_status_bins_workaround(rb_iv_get(self, "@client"), rb_iv_get(self, "@key"), rb_iv_get(self, "@bin_name"));

  return Qtrue;
}

// ----------------------------------------------------------------------------------
//
// def update(value, options = {})
//
static VALUE llist_update(int argc, VALUE * argv, VALUE self) {
  as_error err;
  as_status status;
  aerospike * as = llist_client_struct(self);
  as_key * key   = llist_key_struct(self);

  VALUE value;
  VALUE options;

  rb_scan_args(argc, argv, "11", &value, &options);

  if ( NIL_P(options) ) options = rb_hash_new(); // default options

  as_policy_apply * policy = get_policy(options);
  as_val * val             = rb_val2as_val(value);
  as_ldt * llist           = get_ldt_struct(self);

  if ( ( status = aerospike_llist_update(as, &err, policy, key, llist, val) ) != AEROSPIKE_OK ) {
    as_val_free(val);

    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[AerospikeC::LList][update] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    raise_as_error(err);
  }

  as_val_free(val);

  return Qtrue;
}

// ----------------------------------------------------------------------------------
//
// def update_all(values, options = {})
//
static VALUE llist_update_all(int argc, VALUE * argv, VALUE self) {
  as_error err;
  as_status status;
  aerospike * as = llist_client_struct(self);
  as_key * key   = llist_key_struct(self);

  VALUE values;
  VALUE options;

  rb_scan_args(argc, argv, "11", &values, &options);

  if ( NIL_P(options) ) options = rb_hash_new(); // default options

  as_policy_apply * policy = get_policy(options);
  as_ldt * llist           = get_ldt_struct(self);

  as_arraylist * vals = array2as_list(values);

  if ( ( status = aerospike_llist_update_all(as, &err, policy, key, llist, (as_list *)vals) ) != AEROSPIKE_OK ) {
    as_arraylist_destroy(vals);

    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[AerospikeC::LList][update_all] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    raise_as_error(err);
  }

  as_arraylist_destroy(vals);

  return Qtrue;
}

// ----------------------------------------------------------------------------------
//
// def scan(options = {})
//
static VALUE llist_scan(int argc, VALUE * argv, VALUE self) {
  as_error err;
  as_status status;
  aerospike * as = llist_client_struct(self);
  as_key * key   = llist_key_struct(self);

  VALUE options;

  rb_scan_args(argc, argv, "01", &options);

  if ( NIL_P(options) ) options = rb_hash_new(); // default options

  as_policy_apply * policy = get_policy(options);
  as_ldt * llist           = get_ldt_struct(self);

  as_list * result_list = NULL;

  if ( ( status = aerospike_llist_scan(as, &err, policy, key, llist, &result_list) ) != AEROSPIKE_OK ) {
    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[AerospikeC::LList][scan] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    raise_as_error(err);
  }

  VALUE result = as_list2array((as_arraylist *)result_list);

  as_arraylist_destroy(result_list);

  return result;
}

// ----------------------------------------------------------------------------------
//
// def delete(value, options = {})
//
static VALUE llist_delete(int argc, VALUE * argv, VALUE self) {
  as_error err;
  as_status status;
  aerospike * as = llist_client_struct(self);
  as_key * key   = llist_key_struct(self);

  VALUE value;
  VALUE options;

  rb_scan_args(argc, argv, "11", &value, &options);

  if ( NIL_P(options) ) options = rb_hash_new(); // default options

  as_policy_apply * policy = get_policy(options);
  as_val * val             = rb_val2as_val(value);
  as_ldt * llist           = get_ldt_struct(self);

  if ( ( status = aerospike_llist_remove(as, &err, policy, key, llist, val) ) != AEROSPIKE_OK ) {
    as_val_free(val);

    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[AerospikeC::LList][delete] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    if ( status == AEROSPIKE_ERR_LARGE_ITEM_NOT_FOUND ) {
      log_warn("[AerospikeC::LList][delete] AEROSPIKE_ERR_LARGE_ITEM_NOT_FOUND");
      return Qnil;
    }

    raise_as_error(err);
  }

  as_val_free(val);

  return Qtrue;
}

// ----------------------------------------------------------------------------------
//
// def find(value, options = {})
//
static VALUE llist_find(int argc, VALUE * argv, VALUE self) {
  as_error err;
  as_status status;
  aerospike * as = llist_client_struct(self);
  as_key * key   = llist_key_struct(self);

  VALUE value;
  VALUE options;

  rb_scan_args(argc, argv, "11", &value, &options);

  if ( NIL_P(options) ) options = rb_hash_new(); // default options

  as_policy_apply * policy = get_policy(options);
  as_val * val             = rb_val2as_val(value);
  as_ldt * llist           = get_ldt_struct(self);

  as_list * result_list = NULL;

  if ( ( status = aerospike_llist_find(as, &err, policy, key, llist, val, &result_list) ) != AEROSPIKE_OK ) {
    as_val_free(val);

    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[AerospikeC::LList][find] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    if ( status == AEROSPIKE_ERR_LARGE_ITEM_NOT_FOUND ) {
      log_warn("[AerospikeC::LList][find] AEROSPIKE_ERR_LARGE_ITEM_NOT_FOUND");
      return Qnil;
    }

    raise_as_error(err);
  }

  VALUE result = as_list2array((as_arraylist *)result_list);

  as_val_free(val);
  as_arraylist_destroy(result_list);

  return rb_ary_entry(result, 0);
}

// ----------------------------------------------------------------------------------
//
// def find_first(count, options = {})
//
static VALUE llist_find_first(int argc, VALUE * argv, VALUE self) {
  as_error err;
  as_status status;
  aerospike * as = llist_client_struct(self);
  as_key * key   = llist_key_struct(self);

  VALUE count;
  VALUE options;

  rb_scan_args(argc, argv, "11", &count, &options);

  if ( NIL_P(options) ) options = rb_hash_new(); // default options

  if ( TYPE(count) != T_FIXNUM )
    rb_raise(rb_eRuntimeError, "[AerospikeC::Llist][find_first] count must be integer");

  as_policy_apply * policy = get_policy(options);
  as_ldt * llist           = get_ldt_struct(self);

  as_list * result_list = NULL;

  if ( ( status = aerospike_llist_find_first(as, &err, policy, key, llist, FIX2LONG(count), &result_list) ) != AEROSPIKE_OK ) {
    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[AerospikeC::LList][find_first] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    if ( status == AEROSPIKE_ERR_LARGE_ITEM_NOT_FOUND ) {
      log_warn("[AerospikeC::LList][find_first] AEROSPIKE_ERR_LARGE_ITEM_NOT_FOUND");
      return Qnil;
    }

    raise_as_error(err);
  }

  VALUE result = as_list2array((as_arraylist *)result_list);

  as_arraylist_destroy(result_list);

  return result;
}

// ----------------------------------------------------------------------------------
//
// def first(options = {})
//
static VALUE llist_first(int argc, VALUE * argv, VALUE self) {
  VALUE options;

  rb_scan_args(argc, argv, "01", &options);

  VALUE result = rb_funcall(self, rb_intern("find_first"), 2, INT2FIX(1), options);

  return rb_ary_entry(result, 0);
}

// ----------------------------------------------------------------------------------
//
// def find_last(count, options = {})
//
static VALUE llist_find_last(int argc, VALUE * argv, VALUE self) {
  as_error err;
  as_status status;
  aerospike * as = llist_client_struct(self);
  as_key * key   = llist_key_struct(self);

  VALUE count;
  VALUE options;

  rb_scan_args(argc, argv, "11", &count, &options);

  if ( NIL_P(options) ) options = rb_hash_new(); // default options

  if ( TYPE(count) != T_FIXNUM )
    rb_raise(rb_eRuntimeError, "[AerospikeC::Llist][find_last] count must be integer");

  as_policy_apply * policy = get_policy(options);
  as_ldt * llist           = get_ldt_struct(self);

  as_list * result_list = NULL;

  if ( ( status = aerospike_llist_find_last(as, &err, policy, key, llist, FIX2LONG(count), &result_list) ) != AEROSPIKE_OK ) {
    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[AerospikeC::LList][find_last] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    if ( status == AEROSPIKE_ERR_LARGE_ITEM_NOT_FOUND ) {
      log_warn("[AerospikeC::LList][find_last] AEROSPIKE_ERR_LARGE_ITEM_NOT_FOUND");
      return Qnil;
    }

    raise_as_error(err);
  }

  VALUE result = as_list2array((as_arraylist *)result_list);

  as_arraylist_destroy(result_list);

  return result;
}

// ----------------------------------------------------------------------------------
//
// def last(options = {})
//
static VALUE llist_last(int argc, VALUE * argv, VALUE self) {
  VALUE options;

  rb_scan_args(argc, argv, "01", &options);

  VALUE result = rb_funcall(self, rb_intern("find_last"), 2, INT2FIX(1), options);

  return rb_ary_entry(result, 0);
}

// ----------------------------------------------------------------------------------
//
// def find_from(value, count, options = {})
//
static VALUE llist_find_from(int argc, VALUE * argv, VALUE self) {
  as_error err;
  as_status status;
  aerospike * as = llist_client_struct(self);
  as_key * key   = llist_key_struct(self);

  VALUE value;
  VALUE count;
  VALUE options;

  rb_scan_args(argc, argv, "21", &value, &count, &options);

  if ( NIL_P(options) ) options = rb_hash_new(); // default options

  if ( TYPE(count) != T_FIXNUM )
    rb_raise(rb_eRuntimeError, "[AerospikeC::Llist][find_from] count must be integer");

  as_policy_apply * policy = get_policy(options);
  as_val * val             = rb_val2as_val(value);
  as_ldt * llist           = get_ldt_struct(self);

  as_list * result_list = NULL;

  if ( ( status = aerospike_llist_find_from(as, &err, policy, key, llist, val, FIX2LONG(count), &result_list) ) != AEROSPIKE_OK ) {
    as_val_free(val);

    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[AerospikeC::LList][find_from] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    if ( status == AEROSPIKE_ERR_LARGE_ITEM_NOT_FOUND ) {
      log_warn("[AerospikeC::LList][find_from] AEROSPIKE_ERR_LARGE_ITEM_NOT_FOUND");
      return Qnil;
    }

    raise_as_error(err);
  }

  VALUE result = as_list2array((as_arraylist *)result_list);

  as_val_free(val);
  as_arraylist_destroy(result_list);

  return result;
}

// ----------------------------------------------------------------------------------
//
// def size(options = {})
//
static VALUE llist_size(int argc, VALUE * argv, VALUE self) {
  as_error err;
  as_status status;
  aerospike * as = llist_client_struct(self);
  as_key * key   = llist_key_struct(self);

  VALUE options;

  rb_scan_args(argc, argv, "01", &options);

  if ( NIL_P(options) ) options = rb_hash_new(); // default options

  as_policy_apply * policy = get_policy(options);
  as_ldt * llist           = get_ldt_struct(self);

  uint32_t llist_size = 0;

  if ( ( status = aerospike_llist_size(as, &err, policy, key, llist, &llist_size) ) != AEROSPIKE_OK ) {
    if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
      log_warn("[AerospikeC::LList][size] AEROSPIKE_ERR_RECORD_NOT_FOUND");
      return Qnil;
    }

    raise_as_error(err);
  }

  return LONG2FIX(llist_size);
}

// // ----------------------------------------------------------------------------------
// //
// // def capacity(options = {})
// //
// static VALUE llist_capacity(int argc, VALUE * argv, VALUE self) {
//   as_error err;
//   as_status status;
//   aerospike * as = llist_client_struct(self);
//   as_key * key   = llist_key_struct(self);

//   VALUE options;

//   rb_scan_args(argc, argv, "01", &options);

//   if ( NIL_P(options) ) options = rb_hash_new(); // default options

//   as_policy_apply * policy = get_policy(options);
//   as_ldt * llist           = get_ldt_struct(self);

//   uint32_t ldt_capacity = 0;

//   if ( ( status = aerospike_llist_get_capacity(as, &err, policy, key, llist, &ldt_capacity) ) != AEROSPIKE_OK ) {
//     if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
//       log_warn("[AerospikeC::LList][capacity] AEROSPIKE_ERR_RECORD_NOT_FOUND");
//       return Qnil;
//     }

//     raise_as_error(err);
//   }

//   return LONG2FIX(ldt_capacity);
// }

// // ----------------------------------------------------------------------------------
// //
// // def capacity=(capacity, options = {})
// //
// static VALUE llist_set_capacity(int argc, VALUE * argv, VALUE self) {
//   as_error err;
//   as_status status;
//   aerospike * as = llist_client_struct(self);
//   as_key * key   = llist_key_struct(self);

//   VALUE capacity;
//   VALUE options;

//   rb_scan_args(argc, argv, "11", &capacity, &options);

//   if ( NIL_P(options) ) options = rb_hash_new(); // default options

//   if ( TYPE(capacity) != T_FIXNUM )
//     rb_raise(rb_eRuntimeError, "[AerospikeC::Llist][set_capacity] capacity must be integer");

//   as_policy_apply * policy = get_policy(options);
//   as_ldt * llist           = get_ldt_struct(self);

//   uint32_t ldt_capacity = FIX2LONG(capacity);

//   if ( ( status = aerospike_llist_set_capacity(as, &err, policy, key, llist, &ldt_capacity) ) != AEROSPIKE_OK ) {
//     if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
//       log_warn("[AerospikeC::LList][set_capacity] AEROSPIKE_ERR_RECORD_NOT_FOUND");
//       return Qnil;
//     }

//     raise_as_error(err);
//   }

//   return LONG2FIX(ldt_capacity);
// }

// // ----------------------------------------------------------------------------------
// //
// // def filter=(function_name, udf_args = [], options = {})
// //
// static VALUE llist_filter(int argc, VALUE * argv, VALUE self) {
//   as_error err;
//   as_status status;
//   aerospike * as = llist_client_struct(self);
//   as_key * key   = llist_key_struct(self);

//   VALUE function_name;
//   VALUE udf_args;
//   VALUE options;

//   rb_scan_args(argc, argv, "12", &function_name, &udf_args, &options);

//   // default options
//   if ( NIL_P(options) ) options = rb_hash_new();
//   if ( NIL_P(udf_args) ) udf_args = rb_ary_new();
//   if ( TYPE(function_name) != T_STRING ) function_name = value_to_s(function_name);

//   as_policy_apply * policy = get_policy(options);
//   as_ldt * llist           = get_ldt_struct(self);
//   as_arraylist * args      = array2as_list(udf_args);

//   as_list * result_list = NULL;

//   if ( ( status = aerospike_llist_filter(as, &err, policy, key, llist, StringValueCStr(function_name), (as_list *)args, &result_list) ) != AEROSPIKE_OK ) {
//     as_arraylist_destroy(args);

//     if ( status == AEROSPIKE_ERR_RECORD_NOT_FOUND ) {
//       log_warn("[AerospikeC::LList][set_capacity] AEROSPIKE_ERR_RECORD_NOT_FOUND");
//       return Qnil;
//     }

//     raise_as_error(err);
//   }


//   VALUE result = as_list2array((as_arraylist *)result_list);

//   as_arraylist_destroy(result_list);
//   as_arraylist_destroy(args);

//   return result;
// }

// ----------------------------------------------------------------------------------
//
// Init
//
void init_aerospike_c_llist(VALUE AerospikeC) {
  //
  // class AerospikeC::Llist < Object
  //
  Llist = rb_define_class_under(AerospikeC, "Llist", rb_cObject);

  //
  // methods
  //
  rb_define_method(Llist, "initialize", RB_FN_ANY()llist_initialize, -1);

  rb_define_method(Llist, "add", RB_FN_ANY()llist_add, -1);
  rb_define_method(Llist, "add_all", RB_FN_ANY()llist_add_all, -1);
  rb_define_method(Llist, "update", RB_FN_ANY()llist_update, -1);
  rb_define_method(Llist, "update_all", RB_FN_ANY()llist_update_all, -1);

  rb_define_method(Llist, "scan", RB_FN_ANY()llist_scan, -1);

  rb_define_method(Llist, "delete", RB_FN_ANY()llist_delete, -1);

  rb_define_method(Llist, "find", RB_FN_ANY()llist_find, -1);
  rb_define_method(Llist, "find_first", RB_FN_ANY()llist_find_first, -1);
  rb_define_method(Llist, "first", RB_FN_ANY()llist_first, -1);
  rb_define_method(Llist, "find_last", RB_FN_ANY()llist_find_last, -1);
  rb_define_method(Llist, "last", RB_FN_ANY()llist_last, -1);
  rb_define_method(Llist, "find_from", RB_FN_ANY()llist_find_from, -1);

  rb_define_method(Llist, "size", RB_FN_ANY()llist_size, -1);

  // rb_define_method(Llist, "capacity", RB_FN_ANY()llist_capacity, -1);
  // rb_define_method(Llist, "capacity=", RB_FN_ANY()llist_set_capacity, -1);

  // rb_define_method(Llist, "filter", RB_FN_ANY()llist_filter, -1);

  //
  // attr_reader
  //
  rb_define_attr(Llist, "bin_name", 1, 0);
  rb_define_attr(Llist, "key", 1, 0);
}