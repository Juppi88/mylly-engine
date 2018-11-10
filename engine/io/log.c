#include "log.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

static void log_timestamp(FILE *stream)
{
	time_t local = time(NULL);
	struct tm *tm = localtime(&local);

#ifndef _WIN32
	fprintf(stream, "\033[30;1m"); // Grey
#endif

	fprintf(stream, "[%02u:%02u.%02u] ", tm->tm_hour, tm->tm_min, tm->tm_sec);

#ifndef _WIN32
	fprintf(stream, "\033[37;0m"); // Reset colour
#endif
}

void log_message(const char *note, const char *format, ...)
{
	char buffer[1024];
	va_list args;

	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	log_timestamp(stdout);

	fprintf(stdout, "[%s] %s\n", note, buffer);
}

void log_warning(const char *note, const char *format, ...)
{
	char buffer[1024];
	va_list args;
	
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	log_timestamp(stderr);

#ifndef _WIN32
	fprintf(stderr, "\033[33;1m"); // Set colour to yellow for notification messages.
#endif

	fprintf(stderr, "[%s] %s\n", note, buffer);

#ifndef _WIN32
	fprintf(stderr, "\033[37;0m"); // Reset colour.
#endif
}

void log_error(const char *note, const char *format, ...)
{
	char buffer[1024];
	va_list args;
	
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	log_timestamp(stderr);

#ifndef _WIN32
	fprintf(stderr, "\033[31;1m"); // Set colour to red for error messages.
#endif

	fprintf(stderr, "[%s] %s\n", note, buffer);

#ifndef _WIN32
	fprintf(stderr, "\033[37;0m"); // Reset colour.
#endif
}
