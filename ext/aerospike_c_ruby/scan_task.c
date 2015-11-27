#include <aerospike_c_ruby.h>

VALUE ScanTask;

//
// def initialize(scan_id, client)
//
static VALUE scan_initialize(VALUE self, VALUE scan_id, VALUE client) {
  rb_iv_set(self, "@scan_id", scan_id);
  rb_iv_set(self, "@client", client);

  as_error err;
  aerospike * as = get_client_struct(client);

  uint64_t scanid = FIX2LONG(scan_id);
  as_scan_info scan_info;

  if ( aerospike_scan_info(as, &err, NULL, scanid, &scan_info) != AEROSPIKE_OK ) {
    raise_as_error(err);
  }

  rb_iv_set(self, "@status", INT2FIX(scan_info.status));
  rb_iv_set(self, "@progress", INT2FIX(scan_info.progress_pct));
}

//
// def check_status
//
static VALUE check_status(VALUE self) {
  VALUE client = rb_iv_get(self, "@client");

  as_error err;
  aerospike * as = get_client_struct(client);

  uint64_t scan_id = FIX2LONG(scan_id);
  as_scan_info scan_info;

  if ( aerospike_scan_info(as, &err, NULL, scan_id, &scan_info) != AEROSPIKE_OK ) {
    raise_as_error(err);
  }

  VALUE status = rb_hash_new();

  VALUE stat = INT2FIX(scan_info.status);
  VALUE prog = INT2FIX(scan_info.progress_pct);

  rb_iv_set(self, "@status", stat);
  rb_iv_set(self, "@progress", prog);

  rb_hash_aset(status, rb_str_new2("status"), stat);
  rb_hash_aset(status, rb_str_new2("progress"), prog);

  return status;
}

// ----------------------------------------------------------------------------------
// Init
//
void init_aerospike_c_scan_task(VALUE AerospikeC) {
  //
  // class AerospikeC::ScanTask < Object
  //
  ScanTask = rb_define_class_under(AerospikeC, "ScanTask", rb_cObject);

  //
  // methods
  //
  rb_define_method(ScanTask, "initialize", RB_FN_ANY()scan_initialize, 2);
  rb_define_method(ScanTask, "check_status", RB_FN_ANY()check_status, 0);

  //
  // attr_reader
  //
  rb_define_attr(ScanTask, "scan_id", 1, 0);
  rb_define_attr(ScanTask, "status", 1, 0);
  rb_define_attr(ScanTask, "progress", 1, 0);
}