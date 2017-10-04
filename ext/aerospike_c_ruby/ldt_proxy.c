// @deprecated Starting from version 4.1.6 C Client does not support LDT
// #include <aerospike_c_ruby.h>
//
// VALUE rb_aero_LdtProxy;
//
// static VALUE ldt_proxy_initialize(VALUE self, VALUE client, VALUE key, VALUE bin_name) {
//   rb_iv_set(self, "@bin_name", bin_name);
//   rb_iv_set(self, "@client", client);
//   rb_iv_set(self, "@key", key);
// }
//
// static VALUE get_llist(int argc, VALUE * argv, VALUE self) {
//   VALUE options;
//
//   rb_scan_args(argc, argv, "01", &options);
//
//   return rb_funcall(rb_aero_Llist, rb_intern("new"), 4, rb_iv_get(self, "@client"), rb_iv_get(self, "@key"), rb_iv_get(self, "@bin_name"), options);
// }
//
// // ----------------------------------------------------------------------------------
// //
// // Init
// //
// void init_aerospike_c_ldt_proxy(VALUE AerospikeC) {
//   //
//   // class AerospikeC::LdtProxy < Object
//   //
//   rb_aero_LdtProxy = rb_define_class_under(AerospikeC, "LdtProxy", rb_cObject);
//
//   //
//   // methods
//   //
//   rb_define_method(rb_aero_LdtProxy, "initialize", RB_FN_ANY()ldt_proxy_initialize, 3);
//   rb_define_method(rb_aero_LdtProxy, "llist", RB_FN_ANY()get_llist, -1);
// }
