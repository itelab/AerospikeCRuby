#include <aerospike_c_ruby.h>
#include <client_utils.h>
#include <querying.h>

// ----------------------------------------------------------------------------------
//
// callback method for execute_query
// push into array only if non error
//
// static VALUE execute_query_callback_protected(VALUE rdata) {
//   as_val * val = (as_val *) rdata;

//   as_record * record = as_rec_fromval(val);

//   return record2hash(record);
// }

static bool execute_query_callback(as_val * val, query_item * query_data) {
  if ( val == NULL ) return false;

  query_item * new_item = (query_item *) malloc ( sizeof(query_item) );
  if (! new_item) {
    return true;
  }

  init_query_item(new_item);

  new_item->rec = rb_copy_as_record( as_rec_fromval(val) );

  if (! new_item->rec) {
    return true;
  }

  set_query_item_next(query_data, new_item);

  return true;
}

static void * rb_run_query(query_list * args) {
  if ( aerospike_query_foreach(args->as, &args->err, args->policy, args->query, args->callback, args->query_data) != AEROSPIKE_OK )
    args->is_err = true;

  return NULL;
}

// ----------------------------------------------------------------------------------
//
// execute query
// multiple threads will likely be calling the callback in parallel so return data won't be sorted
//
// def query(query_obj)
//
// params:
//   query_obj - AeropsikeC::Query object
//
//  ------
//  RETURN:
//    1. data returned from query
//
static VALUE execute_query_begin(VALUE rdata) {
  query_list * args = (query_list *) rdata;
  args->is_err = false;

  rb_thread_call_without_gvl(rb_run_query, args, NULL, NULL);

  if ( args->is_err )
    raise_as_error(args->err);

  set_query_result_and_destroy(args);

  return args->result;
}

static VALUE execute_query_ensure(VALUE rdata) {
  query_list * args = (query_list *) rdata;

  as_query_destroy(args->query);
  query_result_destroy(args);

  return args->result;
}

VALUE execute_query(VALUE self, VALUE query_obj) {
  rb_aero_TIMED(tm);

  aerospike * as = rb_aero_CLIENT;

  VALUE is_aerospike_c_query_obj = rb_funcall(query_obj, rb_intern("is_a?"), 1, rb_aero_Query);
  if ( is_aerospike_c_query_obj != Qtrue )
    rb_raise(rb_aero_OptionError, "[AerospikeC::Client][query] use AerospikeC::Query class to perform queries");

  as_query * query         = query_obj2as_query(query_obj);
  as_policy_query * policy = get_query_policy(query_obj);

  query_item * query_data = (query_item *) malloc ( sizeof(query_item) );
  if (! query_data) rb_raise(rb_aero_MemoryError, "[AerospikeC::Client][query] Error while allocating memory for query result");
  init_query_item(query_data);

  query_list q_args;

  q_args.as          = as;
  q_args.policy      = policy;
  q_args.query       = query;
  q_args.query_data  = query_data;
  q_args.callback    = execute_query_callback;
  q_args.result      = rb_ary_new();
  q_args.with_header = false;

  if ( rb_funcall(query_obj, rb_intern("with_header"), 0) == Qtrue ) {
    q_args.with_header = true;
  }

  VALUE result = rb_ensure(execute_query_begin, (VALUE)(&q_args), execute_query_ensure, (VALUE)(&q_args));

  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 1, rb_str_new2("[Client][query] success"));

  return result;
}

// ----------------------------------------------------------------------------------
//
// callback method for execute_udf_on_query
// push into array only if non error
//
static bool execute_udf_on_query_callback(as_val * val, VALUE query_data) {
  if ( val == NULL ) return false;

  query_item * new_item = (query_item *) malloc ( sizeof(query_item) );
  if (! new_item) {
    return true;
  }

  init_query_item(new_item);

  size_t value_type = as_val_type(val);

  if ( value_type == AS_REC ) { // is record
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

  set_query_item_next(query_data, new_item);

  return true;
}

// ----------------------------------------------------------------------------------
//
// execute udf on query
// multiple threads will likely be calling the callback in parallel so return data won't be sorted
//
// def execute_udf_on_query(query_obj, module_name, func_name, udf_args = [])
//
// params:
//   query_obj - AeropsikeC::Query object
//   module_name - string, registered module name
//   func_name - string, function name in module to execute
//   udf_args - arguments passed to udf
//
//  ------
//  RETURN:
//    1. data returned from query
//
static VALUE execute_udf_on_query_begin(VALUE rdata) {
  query_list * args = (query_list *) rdata;
  as_error err;

  if ( aerospike_query_foreach(args->as, &err, args->policy, args->query, args->callback, args->query_data) != AEROSPIKE_OK )
    raise_as_error(err);

  set_query_result_and_destroy(args);

  return args->result;
}

static VALUE execute_udf_on_query_ensure(VALUE rdata) {
  query_list * args = (query_list *) rdata;

  as_query_destroy(args->query);
  // as_arraylist_destroy(args->args);
  query_result_destroy(args);

  return args->result;
}

VALUE execute_udf_on_query(int argc, VALUE * argv, VALUE self)  {
  rb_aero_TIMED(tm);

  aerospike * as = rb_aero_CLIENT;

  VALUE query_obj;
  VALUE module_name;
  VALUE func_name;
  VALUE udf_args;

  rb_scan_args(argc, argv, "31", &query_obj, &module_name, &func_name, &udf_args);

  VALUE is_aerospike_c_query_obj = rb_funcall(query_obj, rb_intern("is_a?"), 1, rb_aero_Query);
  if ( is_aerospike_c_query_obj != Qtrue )
    rb_raise(rb_aero_OptionError, "[AerospikeC::Client][aggregate] use AerospikeC::Query class to perform queries");

  as_arraylist * args = NULL;

  if ( !(NIL_P(udf_args)) ) {
    args = array2as_list(udf_args);
  }

  as_query * query         = query_obj2as_query(query_obj);
  as_policy_query * policy = get_query_policy(query_obj);

  char * c_module_name = StringValueCStr(module_name);
  char * c_func_name = StringValueCStr(func_name);

  as_query_apply(query, c_module_name, c_func_name, (as_list*)args);

  query_item * query_data = (query_item *) malloc ( sizeof(query_item) );
  if (! query_data) rb_raise(rb_aero_MemoryError, "[AerospikeC::Client][query] Error while allocating memory for query result");
  init_query_item(query_data);

  query_list q_args;

  q_args.as         = as;
  q_args.args       = args;
  q_args.policy     = policy;
  q_args.query      = query;
  q_args.query_data = query_data;
  q_args.callback   = execute_udf_on_query_callback;
  q_args.result     = rb_ary_new();

  VALUE result = rb_ensure(execute_udf_on_query_begin, (VALUE)(&q_args), execute_udf_on_query_ensure, (VALUE)(&q_args));

  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 2, rb_str_new2("[Client][aggregate] success"), rb_aero_MOD_INFO);

  return result;
}

// ----------------------------------------------------------------------------------
//
// background execute udf on query
//
// def background_execute_udf_on_query(query_obj, module_name, func_name, udf_args = [])
//
// params:
//   query_obj - AeropsikeC::Query object
//   module_name - string, registered module name
//   func_name - string, function name in module to execute
//   udf_args - arguments passed to udf
//
//  ------
//  RETURN:
//    1. AerospikeC::QueryTask object
//
// @TODO options policy in AeropsikeC::Query
//
VALUE background_execute_udf_on_query(int argc, VALUE * argv, VALUE self)  {
  rb_aero_TIMED(tm);

  as_error err;
  aerospike * as = rb_aero_CLIENT;

  VALUE query_obj;
  VALUE module_name;
  VALUE func_name;
  VALUE udf_args;

  rb_scan_args(argc, argv, "31", &query_obj, &module_name, &func_name, &udf_args);

  VALUE is_aerospike_c_query_obj = rb_funcall(query_obj, rb_intern("is_a?"), 1, rb_aero_Query);
  if ( is_aerospike_c_query_obj != Qtrue ) {
    rb_raise(rb_aero_OptionError, "[AerospikeC::Client][bg_aggregate] use AerospikeC::Query class to perform queries");
  }

  as_arraylist * args = NULL;
  if ( !(NIL_P(udf_args)) ) {
    args = array2as_list(udf_args);
  }

  as_query * query    = query_obj2as_query(query_obj);

  char * c_module_name = StringValueCStr(module_name);
  char * c_func_name = StringValueCStr(func_name);

  as_query_apply(query, c_module_name, c_func_name, (as_list*)args);

  uint64_t query_id = 0;

  if (aerospike_query_background(as, &err, NULL, query, &query_id) != AEROSPIKE_OK) {
    as_query_destroy(query);
    as_arraylist_destroy(args);
    raise_as_error(err);
  }

  as_arraylist_destroy(args);

  VALUE queryid  = ULONG2NUM(query_id);
  VALUE rb_query = Data_Wrap_Struct(rb_aero_Query, NULL, query_task_deallocate, query);

  rb_iv_set(self, "@last_query_id", queryid);

  return rb_funcall(rb_aero_QueryTask, rb_intern("new"), 3, queryid, rb_query, self);
}


// ----------------------------------------------------------------------------------
//
// query each
//
// def query_each(query_obj) {}
//
// params:
//   query_obj - AeropsikeC::Query object
//
//  ------
//  RETURN:
//    1. true on succes
//
// @TODO options policy in AeropsikeC::Query
//
static bool execute_query_each_callback(as_val * val, VALUE blk) {
  pthread_mutex_lock(&G_CALLBACK_MUTEX);

  if ( val == NULL ) {
    pthread_mutex_unlock(&G_CALLBACK_MUTEX);
    return false;
  }

  as_record * rec = as_rec_fromval(val);

  rb_funcall(blk, rb_intern("call"), 1, record2hash(rec));

  pthread_mutex_unlock(&G_CALLBACK_MUTEX);

  return true;
}

VALUE execute_query_each(VALUE self, VALUE query_obj) {
  rb_need_block();

  rb_aero_TIMED(tm);

  aerospike * as = rb_aero_CLIENT;
  as_error err;

  VALUE blk;
  blk = rb_block_proc();

  VALUE is_aerospike_c_query_obj = rb_funcall(query_obj, rb_intern("is_a?"), 1, rb_aero_Query);
  if ( is_aerospike_c_query_obj != Qtrue )
    rb_raise(rb_aero_OptionError, "[AerospikeC::Client][query] use AerospikeC::Query class to perform queries");

  as_query * query         = query_obj2as_query(query_obj);
  as_policy_query * policy = get_query_policy(query_obj);

  if ( aerospike_query_foreach(as, &err, policy, query, execute_query_each_callback, blk) != AEROSPIKE_OK ) {
    as_query_destroy(query);
    raise_as_error(err);
  }

  as_query_destroy(query);

  rb_aero_logger(AS_LOG_LEVEL_DEBUG, &tm, 1, rb_str_new2("[Client][query_each] success"));

  return Qtrue;
}