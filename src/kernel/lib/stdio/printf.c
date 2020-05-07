#include <lib/stdio.h>
#include <stdarg.h>

void printf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}