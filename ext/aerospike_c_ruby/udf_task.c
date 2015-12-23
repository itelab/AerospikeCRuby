#include <aerospike_c_ruby.h>

VALUE UdfTask;

//
// def initialize(name, client)
//
static VALUE udf_initialize(VALUE self, VALUE name, VALUE client) {
  rb_iv_set(self, "@name", name);
  rb_iv_set(self, "@done", Qfalse);
  rb_iv_set(self, "@client", client);
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
  struct timeval tm;
  start_timing(&tm);

  if ( rb_iv_get(self, "@done") == Qtrue ) return Qtrue;

  VALUE interval_ms;

  rb_scan_args(argc, argv, "01", &interval_ms);

  if ( NIL_P(interval_ms) ) interval_ms = rb_zero;

  as_error err;
  aerospike * as = get_client_struct(rb_iv_get(self, "@client"));

  VALUE tmp_name = rb_iv_get(self, "@name");

  if ( aerospike_udf_put_wait(as, &err, NULL, StringValueCStr(tmp_name), FIX2LONG(interval_ms)) != AEROSPIKE_OK ) {
    raise_as_error(err);
  }

  rb_iv_set(self, "@done", Qtrue);
  log_info_with_time("[UdfTask][wait_till_completed] success", &tm);

  return rb_iv_get(self, "@done");
}

// ----------------------------------------------------------------------------------
// Init
//
void init_aerospike_c_udf_task(VALUE AerospikeC) {
  //
  // class AerospikeC::UdfTask < Object
  //
  UdfTask = rb_define_class_under(AerospikeC, "UdfTask", rb_cObject);

  // //
  // // methods
  // //
  rb_define_method(UdfTask, "initialize", RB_FN_ANY()udf_initialize, 2);
  rb_define_method(UdfTask, "done?", RB_FN_ANY()is_done, 0);
  rb_define_method(UdfTask, "wait_till_completed", RB_FN_ANY()wait_till_completed, -1);

  //
  // attr_reader
  //
  rb_define_attr(UdfTask, "name", 1, 0);
  rb_define_attr(UdfTask, "done", 1, 0);
}