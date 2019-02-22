#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void set_log_file(const char *fname);

void info_log(const char *fmt, ...);
void warning_log(const char *fmt, ...);
void error_log(const char *fmt, ...);
void fatal_log(const char *fmt, ...);
void debug_log(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif	// LOGGER_H_
