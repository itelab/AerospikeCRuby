#include <aerospike_c_ruby.h>

VALUE IndexTask;

//
// free memory method
//
void index_task_deallocate(as_index_task * task) {
  free(task);
}

//
// def initialize
//
static VALUE index_initialize(VALUE self, VALUE task) {
  rb_iv_set(self, "@task", task);

  as_index_task * itask;
  Data_Get_Struct(task, as_index_task, itask);

  rb_iv_set(self, "@done", bool2rb_bool(itask->done));
  rb_iv_set(self, "@name", rb_str_new2(itask->name));
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

  as_error err;

  VALUE interval_ms;

  rb_scan_args(argc, argv, "01", &interval_ms);

  if ( NIL_P(interval_ms) ) interval_ms = rb_zero;

  as_index_task * itask;
  Data_Get_Struct(rb_iv_get(self, "@task"), as_index_task, itask);

  if ( aerospike_index_create_wait(&err, itask, FIX2LONG(interval_ms)) != AEROSPIKE_OK ) {
    raise_as_error(err);
  }

  rb_iv_set(self, "@done", bool2rb_bool(itask->done));

  return Qtrue;
}


// ----------------------------------------------------------------------------------
// Init
//
void init_aerospike_c_index_task(VALUE AerospikeC) {
  //
  // class AerospikeC::IndexTask < Object
  //
  IndexTask = rb_define_class_under(AerospikeC, "IndexTask", rb_cObject);

  //
  // methods
  //
  rb_define_method(IndexTask, "initialize", RB_FN_ANY()index_initialize, 1);
  rb_define_method(IndexTask, "done?", RB_FN_ANY()is_done, 0);
  rb_define_method(IndexTask, "wait_till_completed", RB_FN_ANY()wait_till_completed, -1);

  //
  // attr_reader
  //
  rb_define_attr(IndexTask, "name", 1, 0);
  rb_define_attr(IndexTask, "done", 1, 0);
}