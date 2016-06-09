#include <aerospike_c_ruby.h>
#include <client_utils.h>
#include <scan.h>

// ----------------------------------------------------------------------------------
//
// callback for scan_records
// push into array only if non error
//
static bool scan_records_callback(const as_val * val, query_item * scan_data) {
  if ( val == NULL ) return false;

  query_item * new_item = (query_item *) malloc ( sizeof(query_item) );
  if (! new_item) {
    return true;
  }

  init_query_item(new_item);

  if ( as_val_type(val) == AS_REC ) { // is record
    new_item->rec = rb_copy_as_record( as_rec_fromval(val) );

    if (! new_item->rec) {
      return true;
    }
  }
  else { // is value
    as_val_reserve(val);
    new_item->val = val;

    if (! new_item->val) {
      return true;
    }
  }

  set_query_item_next(scan_data, new_item);

  return true;
}

// ----------------------------------------------------------------------------------
//
// scan records in specified namespace and set
// multiple threads will likely be calling the callback in parallel so return data won't be sorted
//
// def scan(ns, set, options = {})
//
// params:
//   ns - namespace to scan
//   set - set to scan
//   options:
//     - priority - scan priority
//
//  ------
//  RETURN:
//    1. data returned from scan
//
// @TODO options policy
//
static VALUE scan_records_begin(VALUE rdata) {
  scan_list * args = (scan_list *) rdata;
  as_error err;


  if ( aerospike_scan_foreach(args->as, &err, args->policy, args->scan, args->callback, args->scan_data) != AEROSPIKE_OK )
    raise_as_error(err);

  set_scan_result_and_destroy(args);

  return args->result;
}

static VALUE scan_records_ensure(VALUE rdata) {
  scan_list * args = (scan_list *) rdata;

  as_scan_destroy(args->scan);
  scan_result_destroy(args);

  return args->result;
}

VALUE scan_records(int argc, VALUE * argv, VALUE self) {
  rb_aero_TIMED(tm);

  aerospike * as = rb_aero_CLIENT;

  VALUE ns;
  VALUE set;
  VALUE options;

  rb_scan_args(argc, argv, "21", &ns, &set, &options);

  // default values for optional arguments
  if ( NIL_P(options) ) {
    options = rb_hash_new();
  }

  as_scan * scan = as_scan_new(StringValueCStr(ns), StringValueCStr(set));
  set_priority_options(scan, options);

  query_item * scan_data = (query_item *) malloc ( sizeof(query_item) );
  if (! scan_data) rb_raise(rb_aero_MemoryError, "[AerospikeC::Client][query] Error while allocating memory for query result");
  init_query_item(scan_data);

  scan_list s_args;

  s_args.as        = as;
  s_args.scan      = scan;
  s_args.policy    = NULL;
  s_args.callback  = scan_records_callback;
  s_args.scan_data = scan_data;
  s_args.result    = rb_ary_new();

  VALUE result = rb_ensure(scan_records_begin, (VALUE)(&s_args), scan_records_ensure, (VALUE)(&s_args));;

  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 1, rb_str_new2("[Client][scan] success"));

  return result;
}


// ----------------------------------------------------------------------------------
//
// execute udf on scan records in specified namespace and set
// multiple threads will likely be calling the callback in parallel so return data won't be sorted
//
// def execute_udf_on_scan(ns, set, module_name, func_name, udf_args = [], options = {})
//
// params:
//   ns - namespace to scan
//   set - set to scan
//   module_name - string, registered module name
//   func_name - string, function name in module to execute
//   udf_args - arguments passed to udf
//   options:
//     - priority - scan priority
//
//  ------
//  RETURN:
//    1. data returned from scan
//
// @TODO options policy
//
static VALUE execute_udf_on_scan_begin(VALUE rdata) {
  scan_list * args = (scan_list *) rdata;
  as_error err;


  if ( aerospike_scan_foreach(args->as, &err, args->policy, args->scan, args->callback, args->scan_data) != AEROSPIKE_OK )
    raise_as_error(err);

  set_scan_result_and_destroy(args);

  return args->result;
}

static VALUE execute_udf_on_scan_ensure(VALUE rdata) {
  scan_list * args = (scan_list *) rdata;

  as_scan_destroy(args->scan);
  as_arraylist_destroy(args->args);
  scan_result_destroy(args);

  return args->result;
}

VALUE execute_udf_on_scan(int argc, VALUE * argv, VALUE self) {
  rb_aero_TIMED(tm);

  aerospike * as = rb_aero_CLIENT;

  VALUE ns;
  VALUE set;
  VALUE module_name;
  VALUE func_name;
  VALUE udf_args;
  VALUE options;

  rb_scan_args(argc, argv, "42", &ns, &set, &module_name, &func_name, &udf_args, &options);

  // default values for optional arguments
  if ( NIL_P(options) ) options = rb_hash_new();

  as_arraylist * args = NULL;
  if ( !(NIL_P(udf_args)) ) {
    args = array2as_list(udf_args);
  }

  char * c_module_name = StringValueCStr(module_name);
  char * c_func_name = StringValueCStr(func_name);

  as_scan * scan = as_scan_new(StringValueCStr(ns), StringValueCStr(set));
  as_scan_apply_each(scan, c_module_name, c_func_name, (as_list *)args);
  set_priority_options(scan, options);

  query_item * scan_data = (query_item *) malloc ( sizeof(query_item) );
  if (! scan_data) rb_raise(rb_aero_MemoryError, "[AerospikeC::Client][query] Error while allocating memory for query result");
  init_query_item(scan_data);

  scan_list s_args;

  s_args.as        = as;
  s_args.scan      = scan;
  s_args.policy    = NULL;
  s_args.callback  = scan_records_callback;
  s_args.scan_data = scan_data;
  s_args.args      = args;
  s_args.result    = rb_ary_new();


  VALUE result = rb_ensure(scan_records_begin, (VALUE)(&s_args), scan_records_ensure, (VALUE)(&s_args));;

  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 2, rb_str_new2("[Client][scan_udf] success"), rb_aero_MOD_INFO);

  return result;
}

// ----------------------------------------------------------------------------------
//
// execute udf on scan records in specified namespace and set in background
//
// def background_execute_udf_on_scan(ns, set, module_name, func_name, udf_args = [], options = {})
//
// params:
//   ns - namespace to scan
//   set - set to scan
//   module_name - string, registered module name
//   func_name - string, function name in module to execute
//   udf_args - arguments passed to udf
//   options:
//     - priority: scan priority
//     - policy: AerospikeC::Policy for write
//
//  ------
//  RETURN:
//    1. AerospikeC::ScanTask object
//
VALUE background_execute_udf_on_scan(int argc, VALUE * argv, VALUE self) {
  rb_aero_TIMED(tm);

  as_error err;
  aerospike * as = rb_aero_CLIENT;

  VALUE ns;
  VALUE set;
  VALUE module_name;
  VALUE func_name;
  VALUE udf_args;
  VALUE options;

  rb_scan_args(argc, argv, "42", &ns, &set, &module_name, &func_name, &udf_args, &options);

  // default values for optional arguments
  if ( NIL_P(options) ) options = rb_hash_new();

  as_arraylist * args = NULL;
  if ( !(NIL_P(udf_args)) ) {
    args = array2as_list(udf_args);
  }

  as_policy_write * policy = get_policy(options);

  as_scan scan;
  as_scan_init(&scan, StringValueCStr(ns), StringValueCStr(set));

  char * c_module_name = StringValueCStr(module_name);
  char * c_func_name = StringValueCStr(func_name);

  as_scan_apply_each(&scan, c_module_name, c_func_name, (as_list *)args);
  set_priority_options(&scan, options);

  uint64_t scanid = 0;

  if ( aerospike_scan_background(as, &err, policy, &scan, &scanid) != AEROSPIKE_OK )
    raise_as_error(err);

  VALUE scan_id = ULONG2NUM(scanid);

  rb_iv_set(self, "@last_scan_id", scan_id);

  as_scan_destroy(&scan);
  as_arraylist_destroy(args);

  return rb_funcall(rb_aero_ScanTask, rb_intern("new"), 2, scan_id, self);
}



// ----------------------------------------------------------------------------------
//
// callback for scan_records
// push into array only if non error
//
static bool scan_each_callback(const as_val * val, VALUE blk) {
  pthread_mutex_lock(&G_CALLBACK_MUTEX);

  if ( val == NULL ) return false;

  as_record * rec = as_rec_fromval(val);

  rb_funcall(blk, rb_intern("call"), 1, record2hash(rec));

  return true;

  pthread_mutex_unlock(&G_CALLBACK_MUTEX);
}


VALUE scan_each(int argc, VALUE * argv, VALUE self) {
  rb_aero_TIMED(tm);

  aerospike * as = rb_aero_CLIENT;
  as_error err;

  VALUE ns;
  VALUE set;
  VALUE options;
  VALUE blk;

  rb_scan_args(argc, argv, "21&", &ns, &set, &options, &blk);

  // default values for optional arguments
  if ( NIL_P(options) ) {
    options = rb_hash_new();
  }

  as_scan * scan = as_scan_new(StringValueCStr(ns), StringValueCStr(set));
  set_priority_options(scan, options);


  if ( aerospike_scan_foreach(as, &err, NULL, scan, scan_each_callback, blk) != AEROSPIKE_OK )
    raise_as_error(err);


  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 1, rb_str_new2("[Client][scan_each] success"));

  return Qtrue;
}