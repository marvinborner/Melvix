#include <stdarg.h>
#include <lib/stdio.h>

void printf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}