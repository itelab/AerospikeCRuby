#include <aerospike_c_ruby.h>

static int color_code = 35;

static void switch_color_code() {
  if (color_code == 35) color_code = 36;
  else if (color_code == 36) color_code = 35;
  else color_code = 36;
}

// ----------------------------------------------------------------------------------
//
// logger methods
//
bool rb_aero_log_callback(
    as_log_level level, const char * func, const char * file, uint32_t line,
    const char * fmt, ...)
{
      VALUE logger = rb_funcall(rb_aero_AerospikeC, rb_intern("logger"), 0);

  if ( TYPE(logger) != T_OBJECT ) return;

    char msg[1024] = {0};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, 1024, fmt, ap);
    msg[1023] = '\0';
    va_end(ap);

    switch_color_code();

    VALUE rb_msg = rb_sprintf("\e[1m\e[%dm%s\e[0m [%s:%d][%s] %d - %s", color_code, "<AerospikeC>", file, line, func, level, msg);
    rb_funcall(logger, rb_intern("debug"), 1, rb_msg);

    return true;
}


bool rb_aero_logger(as_log_level level, struct timeval * tm, int args, ...) {
  VALUE logger = rb_funcall(rb_aero_AerospikeC, rb_intern("logger"), 0);

  if ( TYPE(logger) != T_OBJECT ) return;

  switch_color_code();

  VALUE msg = rb_sprintf("\e[1m\e[%dm<AerospikeC>", color_code);

  if ( tm != NULL ) {
    struct timeval tm2;
    gettimeofday(&tm2, NULL);

    double elapsedTime = (tm2.tv_sec - tm->tv_sec) * 1000.0;
    elapsedTime += (tm2.tv_usec - tm->tv_usec) / 1000.0;

    VALUE rb_time = rb_sprintf(" (%.4f ms)", elapsedTime);

    rb_funcall(msg, rb_intern("<<"), 1, rb_time);
  }

  rb_funcall(msg, rb_intern("<<"), 1, rb_str_new2("\e[0m"));

  va_list ap;
  va_start(ap, args);

  VALUE val;

  for (int i = 0; i < args; ++i) {
    val = va_arg(ap, VALUE);

    rb_funcall(msg, rb_intern("<<"), 1, rb_str_new2(" "));
    rb_funcall(msg, rb_intern("<<"), 1, val);
  }

  va_end(ap);

  ID method;

  switch( level ) {
    case AS_LOG_LEVEL_ERROR:
      method = rb_intern("error");
      break;

    case AS_LOG_LEVEL_WARN:
      method = rb_intern("warn");
      break;

    case AS_LOG_LEVEL_INFO:
      method = rb_intern("info");
      break;

    case AS_LOG_LEVEL_DEBUG:
      method = rb_intern("debug");
      break;
  }

  rb_funcall(logger, method, 1, msg);
}
