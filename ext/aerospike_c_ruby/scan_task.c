#include <aerospike_c_ruby.h>

VALUE rb_aero_ScanTask;

VALUE status2ruby_str(as_scan_status status) {
  if ( status == AS_SCAN_STATUS_UNDEF ) {
    return rb_str_new2("AS_SCAN_STATUS_UNDEF");
  }
  else if ( status == AS_SCAN_STATUS_INPROGRESS ) {
    return rb_str_new2("AS_SCAN_STATUS_INPROGRESS");
  }
  else if ( status == AS_SCAN_STATUS_ABORTED ) {
    return rb_str_new2("AS_SCAN_STATUS_ABORTED");
  }
  else if ( status == AS_SCAN_STATUS_COMPLETED ) {
    return rb_str_new2("AS_SCAN_STATUS_COMPLETED");
  }
  else {
    rb_raise(rb_aero_ParseError, "[AerospikeC::ScanTask][status] undefined ScanTask status");
  }
}

//
// def initialize(scan_id, client)
//
static VALUE scan_initialize(VALUE self, VALUE scan_id, VALUE client) {
  rb_iv_set(self, "@scan_id", scan_id);
  rb_iv_set(self, "@client", client);

  as_error err;
  aerospike * as = get_client_struct(client);

  uint64_t scanid = NUM2ULONG(scan_id);
  as_scan_info scan_info;

  if ( aerospike_scan_info(as, &err, NULL, scanid, &scan_info) != AEROSPIKE_OK ) {
    raise_as_error(err);
  }

  rb_iv_set(self, "@status", status2ruby_str(scan_info.status));
  rb_iv_set(self, "@status_id", INT2FIX(scan_info.status));
  rb_iv_set(self, "@progress", INT2FIX(scan_info.progress_pct));
}

//
// def check_status
//
static VALUE check_status(VALUE self) {
  struct timeval tm;
  start_timing(&tm);

  VALUE client = rb_iv_get(self, "@client");

  as_error err;
  aerospike * as = get_client_struct(client);

  uint64_t scan_id = NUM2ULONG( rb_iv_get(self, "@scan_id") );
  as_scan_info scan_info;

  if ( aerospike_scan_info(as, &err, NULL, scan_id, &scan_info) != AEROSPIKE_OK ) {
    raise_as_error(err);
  }

  VALUE status = rb_hash_new();

  VALUE stat    = status2ruby_str(scan_info.status);
  VALUE prog    = INT2FIX(scan_info.progress_pct);
  VALUE stat_id = INT2FIX(scan_info.status);

  rb_iv_set(self, "@status", stat);
  rb_iv_set(self, "@status_id", stat_id);
  rb_iv_set(self, "@progress", prog);

  rb_hash_aset(status, rb_str_new2("status"), stat);
  rb_hash_aset(status, rb_str_new2("status_id"), stat_id);
  rb_hash_aset(status, rb_str_new2("progress"), prog);

  log_info_with_time("[ScanTask][check_status] success", &tm);

  return status;
}

//
// def completed?
//
static VALUE is_completed(VALUE self) {
  int status = FIX2INT( rb_iv_get(self, "@status_id") );

  if ( status == AS_SCAN_STATUS_COMPLETED ) return Qtrue;

  return Qfalse;
}

// ----------------------------------------------------------------------------------
//
// Init
//
void init_aerospike_c_scan_task(VALUE AerospikeC) {
  //
  // class AerospikeC::ScanTask < Object
  //
  rb_aero_ScanTask = rb_define_class_under(AerospikeC, "ScanTask", rb_cObject);

  //
  // consts
  //
  rb_define_const(rb_aero_ScanTask, "AS_SCAN_PRIORITY_AUTO",    INT2FIX(AS_SCAN_PRIORITY_AUTO));
  rb_define_const(rb_aero_ScanTask, "AS_SCAN_PRIORITY_LOW",     INT2FIX(AS_SCAN_PRIORITY_LOW));
  rb_define_const(rb_aero_ScanTask, "AS_SCAN_PRIORITY_MEDIUM",  INT2FIX(AS_SCAN_PRIORITY_MEDIUM));
  rb_define_const(rb_aero_ScanTask, "AS_SCAN_PRIORITY_HIGH",    INT2FIX(AS_SCAN_PRIORITY_HIGH));

  //
  // methods
  //
  rb_define_method(rb_aero_ScanTask, "initialize", RB_FN_ANY()scan_initialize, 2);
  rb_define_method(rb_aero_ScanTask, "check_status", RB_FN_ANY()check_status, 0);
  rb_define_method(rb_aero_ScanTask, "completed?", RB_FN_ANY()is_completed, 0);

  //
  // attr_reader
  //
  rb_define_attr(rb_aero_ScanTask, "scan_id", 1, 0);
  rb_define_attr(rb_aero_ScanTask, "status", 1, 0);
  rb_define_attr(rb_aero_ScanTask, "status_id", 1, 0);
  rb_define_attr(rb_aero_ScanTask, "progress", 1, 0);
}