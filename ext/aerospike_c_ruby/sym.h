#ifndef AEROSPIKE_C_RUBY_SYM_H
#define AEROSPIKE_C_RUBY_SYM_H

#define with_header_sym         ID2SYM(rb_intern("with_header"))        // :with_header
#define ttl_sym                 ID2SYM(rb_intern("ttl"))                // :ttl
#define write_sym               ID2SYM(rb_intern("write"))              // :write
#define read_sym                ID2SYM(rb_intern("read"))               // :read
#define increment_sym           ID2SYM(rb_intern("increment"))          // :increment
#define append_sym              ID2SYM(rb_intern("append"))             // :append
#define prepend_sym             ID2SYM(rb_intern("prepend"))            // :prepend
#define touch_sym               ID2SYM(rb_intern("touch"))              // :touch
#define operation_sym           ID2SYM(rb_intern("operation"))          // :operation
#define bin_sym                 ID2SYM(rb_intern("bin"))                // :bin
#define value_sym               ID2SYM(rb_intern("value"))              // :value
#define numeric_sym             ID2SYM(rb_intern("numeric"))            // :numeric
#define string_sym              ID2SYM(rb_intern("string"))             // :string
#define lua_sym                 ID2SYM(rb_intern("lua"))                // :lua
#define name_sym                ID2SYM(rb_intern("name"))               // :name
#define udf_type_sym            ID2SYM(rb_intern("udf_type"))           // :udf_type
#define hash_sym                ID2SYM(rb_intern("hash"))               // :hash
#define size_sym                ID2SYM(rb_intern("size"))               // :size
#define type_sym                ID2SYM(rb_intern("type"))               // :type
#define filter_type_sym         ID2SYM(rb_intern("filter_type"))        // :filter_type
#define eql_sym                 ID2SYM(rb_intern("eql"))                // :eql
#define range_sym               ID2SYM(rb_intern("range"))              // :range
#define min_sym                 ID2SYM(rb_intern("min"))                // :min
#define max_sym                 ID2SYM(rb_intern("max"))                // :max
#define asc_sym                 ID2SYM(rb_intern("asc"))                // :asc
#define desc_sym                ID2SYM(rb_intern("desc"))               // :desc
#define order_sym               ID2SYM(rb_intern("order"))              // :order
#define at_sym                  ID2SYM(rb_intern("at"))                 // :at
#define c_log_sym               ID2SYM(rb_intern("c_log"))              // :c_log
;
#define lua_path_sym            ID2SYM(rb_intern("lua_path"))           // :lua_path
#define lua_system_path_sym     ID2SYM(rb_intern("lua_system_path"))    // :lua_system_path
#define password_sym            ID2SYM(rb_intern("password"))           // :password
#define logger_sym              ID2SYM(rb_intern("logger"))             // :logger
#define hosts_sym               ID2SYM(rb_intern("hosts"))              // :hosts
#define host_sym                ID2SYM(rb_intern("host"))               // :host
#define port_sym                ID2SYM(rb_intern("port"))               // :port
#define interval_sym            ID2SYM(rb_intern("interval"))           // :interval
#define user_sym                ID2SYM(rb_intern("user"))               // :user
#define thread_pool_size_sym    ID2SYM(rb_intern("thread_pool_size"))   // :thread_pool_size
#define max_threads_sym         ID2SYM(rb_intern("max_threads"))        // :max_threads
#define conn_timeout_sym        ID2SYM(rb_intern("conn_timeout"))       // :conn_timeout
#define fail_not_connected_sym  ID2SYM(rb_intern("fail_not_connected")) // :fail_not_connected
#define commit_level_sym        ID2SYM(rb_intern("commit_level"))       // :commit_level
#define exists_sym              ID2SYM(rb_intern("exists"))             // :exists
#define gen_sym                 ID2SYM(rb_intern("gen"))                // :gen
#define key_sym                 ID2SYM(rb_intern("key"))                // :key
#define retry_sym               ID2SYM(rb_intern("retry"))              // :retry
#define timeout_sym             ID2SYM(rb_intern("timeout"))            // :timeout
#define consistency_level_sym   ID2SYM(rb_intern("consistency_level"))  // :consistency_level
#define replica_sym             ID2SYM(rb_intern("replica"))            // :replica
#define remove_sym              ID2SYM(rb_intern("remove"))             // :remove
#define generation_sym          ID2SYM(rb_intern("generation"))         // :generation
#define policy_sym              ID2SYM(rb_intern("policy"))             // :policy
#define apply_sym               ID2SYM(rb_intern("apply"))              // :apply
#define priority_sym            ID2SYM(rb_intern("priority"))           // :priority
#define query_sym               ID2SYM(rb_intern("query"))              // :query
#define module_sym              ID2SYM(rb_intern("module"))             // :module
#define ldt_proxy_sym           ID2SYM(rb_intern("ldt_proxy"))          // :ldt_proxy
#define geo_json_sym            ID2SYM(rb_intern("geo_json"))           // :geo_json
#define geo_contains_sym        ID2SYM(rb_intern("geo_contains"))       // :geo_contains
#define geo_within_sym          ID2SYM(rb_intern("geo_within"))         // :geo_within
#define geo_sphere_sym          ID2SYM(rb_intern("geo_sphere"))         // :geo_sphere
#define cost_sym                ID2SYM(rb_intern("cost"))               // :cost
#define count_sym               ID2SYM(rb_intern("count"))              // :count
#define operate_sym             ID2SYM(rb_intern("operate"))            // :operate
;

#define list_append_sym         ID2SYM(rb_intern("list_append"))        // :list_append
#define list_set_sym            ID2SYM(rb_intern("list_set"))           // :list_set
#define list_trim_sym           ID2SYM(rb_intern("list_trim"))          // :list_trim
#define list_clear_sym          ID2SYM(rb_intern("list_clear"))         // :list_clear
#define list_size_sym           ID2SYM(rb_intern("list_size"))          // :list_size
#define list_pop_sym            ID2SYM(rb_intern("list_pop"))           // :list_pop
#define list_pop_range_sym      ID2SYM(rb_intern("list_pop_range"))     // :list_pop_range
#define list_pop_range_from_sym ID2SYM(rb_intern("list_pop_range_from"))// :list_pop_range_from
#define list_remove_sym            ID2SYM(rb_intern("list_remove"))           // :list_remove
#define list_remove_range_sym      ID2SYM(rb_intern("list_remove_range"))     // :list_remove_range
#define list_remove_range_from_sym ID2SYM(rb_intern("list_remove_range_from"))// :list_remove_range_from
#define list_get_sym            ID2SYM(rb_intern("list_get"))           // :list_get
#define list_get_range_sym      ID2SYM(rb_intern("list_get_range"))     // :list_get_range
#define list_get_range_from_sym ID2SYM(rb_intern("list_get_range_from"))// :list_get_range_from
;

#define predexp_sym                   ID2SYM(rb_intern("predexp"))   // :predexp
#define predexp_equal_sym             ID2SYM(rb_intern("equal"))     // :equal
#define predexp_unequal_sym           ID2SYM(rb_intern("unequal"))   // :unequal
#define predexp_greater_sym           ID2SYM(rb_intern("greater"))   // :greater
#define predexp_greatereq_sym         ID2SYM(rb_intern("greatereq")) // :greatereq
#define predexp_less_sym              ID2SYM(rb_intern("less"))      // :less
#define predexp_lesseq_sym            ID2SYM(rb_intern("lesseq"))    // :lesseq
#define predexp_within_sym            ID2SYM(rb_intern("within"))    // :within
#define predexp_contains_sym          ID2SYM(rb_intern("contains"))  // :contains
;

#endif // AEROSPIKE_C_RUBY_SYM_H
