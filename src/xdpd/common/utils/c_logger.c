#include "c_logger.h"

enum xdpd_debug_levels xdpd_debug_level[MAX_DEBUG_CLASS] = XDPD_DEFAULT_LEVELS;
int (*xdpd_debug_print)(FILE *stream, const char *format, ...) = &fprintf;

//API to set the logging function
void xdpd_set_logging_function(int (*logging_func)(FILE *stream, const char *format, ...)){
	xdpd_debug_print = logging_func;
}

void xdpd_set_logging_level(/*cn,*/ enum xdpd_debug_levels level){
	xdpd_debug_level[DEFAULT] = level;
}
