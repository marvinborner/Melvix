#include <lib/stdio.h>
#include <stdarg.h>

int sprintf(char *str, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int ret = vsprintf(str, fmt, args);
	va_end(args);
	return ret;
}