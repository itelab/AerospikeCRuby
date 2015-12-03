#include <aerospike_c_ruby.h>

VALUE QueryTask;

void query_task_deallocate(as_query * query) {
  destroy_query(query);
}

//
// def initialize(query_id, query, client)
//
static VALUE query_initialize(VALUE self, VALUE query_id, VALUE query, VALUE client) {
  rb_iv_set(self, "@query_id", query_id);
  rb_iv_set(self, "@query", query);
  rb_iv_set(self, "@client", client);
  rb_iv_set(self, "@done", Qfalse);
}

//
// def done?
//
static VALUE is_done(VALUE self) {
  return rb_iv_get(self, "@done");
}

//
// wait_till_completed(interval_ms = 0)
//
// params:
//   interval_ms - the polling interval in milliseconds. If zero, 1000 ms is used
//
static VALUE wait_till_completed(int argc, VALUE * argv, VALUE self) {
  if ( rb_iv_get(self, "@done") == Qtrue ) return Qtrue;

  VALUE interval_ms;

  rb_scan_args(argc, argv, "01", &interval_ms);

  if ( NIL_P(interval_ms) ) interval_ms = rb_zero;

  as_error err;
  aerospike * as   = get_client_struct(rb_iv_get(self, "@client"));

  as_query * query;
  Data_Get_Struct(rb_iv_get(self, "@query"), as_query, query);

  uint64_t query_id = NUM2ULONG( rb_iv_get(self, "@query_id") );

  if ( aerospike_query_wait(as, &err, NULL, query, query_id, FIX2LONG(interval_ms)) != AEROSPIKE_OK ) {
    raise_as_error(err);
  }

  rb_iv_set(self, "@done", Qtrue);
  log_info("[AerospikeC::QueryTask][wait_till_completed] success");

  return rb_iv_get(self, "@done");
}

// ----------------------------------------------------------------------------------
// Init
//
void init_aerospike_c_query_task(VALUE AerospikeC) {
  //
  // class AerospikeC::QueryTask < Object
  //
  QueryTask = rb_define_class_under(AerospikeC, "QueryTask", rb_cObject);

  // //
  // // methods
  // //
  rb_define_method(QueryTask, "initialize", RB_FN_ANY()query_initialize, 3);
  rb_define_method(QueryTask, "done?", RB_FN_ANY()is_done, 0);
  rb_define_method(QueryTask, "wait_till_completed", RB_FN_ANY()wait_till_completed, -1);

  //
  // attr_reader
  //
  rb_define_attr(Key, "name", 1, 0);
  rb_define_attr(Key, "done", 1, 0);
}