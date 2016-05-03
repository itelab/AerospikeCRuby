#ifndef AEROSPIKE_C_RUBY_LOGGER_H
#define AEROSPIKE_C_RUBY_LOGGER_H


bool rb_aero_log_callback(
    as_log_level level, const char * func, const char * file, uint32_t line,
    const char * fmt, ...);

bool rb_aero_logger(as_log_level level, struct timeval * tm, int args, ...);


#endif // AEROSPIKE_C_RUBY_LOGGER_H