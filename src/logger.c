#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "logger.h"
#include "osd.h"

#if defined(unix) || defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#elif defined(WIN32)
#include <windows.h>
#endif

#define UI_MSG_TIMEOUT 4000

enum { LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_FATAL, LOG_DEBUG };

static int typecolor(int type);
static const char *typeprefix(int type);

static FILE *fp;
static FILE *logfile;

static void logmsg(int type, const char *fmt, va_list ap)
{
	va_list ap_orig;

	if(!fp) fp = stdout;

	va_copy(ap_orig, ap);

#if defined(unix) || defined(__unix__) || (defined(__APPLE__) && !defined(TARGET_IPHONE))
	if(isatty(fileno(fp)) && type != LOG_INFO) {
		int c = typecolor(type);
		fprintf(fp, "\033[%dm", c);
		vfprintf(fp, fmt, ap);
		fprintf(fp, "\033[0m");
	} else
#endif
	{
		fprintf(fp, "%s", typeprefix(type));
		vfprintf(fp, fmt, ap);
	}
	if(type == LOG_ERROR || type == LOG_FATAL || type == LOG_DEBUG) {
		fflush(fp);
	}

	if(logfile) {
		va_end(ap);
		va_copy(ap, ap_orig);
		fprintf(logfile, "%s", typeprefix(type));
		vfprintf(logfile, fmt, ap);
	}

#ifdef WIN32
	if(type == LOG_FATAL) {
		static char msgbuf[1024];
		vsnprintf(msgbuf, sizeof msgbuf - 1, fmt, ap_orig);
		msgbuf[sizeof msgbuf - 1] = 0;
		MessageBox(0, msgbuf, "Fatal error", MB_OK | MB_ICONSTOP);
	}
#endif

	va_end(ap_orig);
}

static void close_logfile(void)
{
	if(logfile) fclose(logfile);
}

void set_log_file(const char *fname)
{
	static int init_once;

	close_logfile();
	logfile = fopen(fname, "w");

	if(!init_once) {
		atexit(close_logfile);
		init_once = 1;
	}
}

void info_log(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	logmsg(LOG_INFO, fmt, ap);
	va_end(ap);
}

void warning_log(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	logmsg(LOG_WARNING, fmt, ap);
	va_end(ap);

	va_start(ap, fmt);
	show_messagev(UI_MSG_TIMEOUT, 1.0, 0.8, 0.1, fmt, ap);
	va_end(ap);
}

void error_log(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	logmsg(LOG_ERROR, fmt, ap);
	va_end(ap);

	va_start(ap, fmt);
	show_messagev(UI_MSG_TIMEOUT, 1.0, 0.1, 0.1, fmt, ap);
	va_end(ap);
}

void fatal_log(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	logmsg(LOG_FATAL, fmt, ap);
	va_end(ap);
}

void debug_log(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	logmsg(LOG_DEBUG, fmt, ap);
	va_end(ap);
}

enum {
	BLACK = 0,
	RED,
	GREEN,
	YELLOW,
	BLUE,
	MAGENTA,
	CYAN,
	WHITE
};

#define ANSI_FGCOLOR(x)	(30 + (x))
#define ANSI_BGCOLOR(x)	(40 + (x))

static int typecolor(int type)
{
	switch(type) {
	case LOG_ERROR:
		return ANSI_FGCOLOR(RED);
	case LOG_FATAL:
		return ANSI_FGCOLOR(RED);	// TODO differentiate from LOG_ERROR
	case LOG_WARNING:
		return ANSI_FGCOLOR(YELLOW);
	case LOG_DEBUG:
		return ANSI_FGCOLOR(MAGENTA);
	default:
		break;
	}
	return 37;
}

static const char *typeprefix(int type)
{
	switch(type) {
	case LOG_ERROR:
		return "E: ";
	case LOG_FATAL:
		return "F: ";
	case LOG_WARNING:
		return "W: ";
	case LOG_DEBUG:
		return "D: ";
	default:
		break;
	}
	return "";
}
