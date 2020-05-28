#include <io/io.h>
#include <lib/lib.h>
#include <lib/stdio.h>
#include <lib/stdlib.h>
#include <stdint.h>

void serial_print(const char *data)
{
	for (u32 i = 0; i < strlen(data); i++)
		serial_put(data[i]);
}

void serial_vprintf(const char *fmt, va_list args)
{
	char buf[1024];
	memset(buf, 0, 1024);
	vsprintf(buf, fmt, args);
	serial_print(buf);
}

void serial_printf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	serial_vprintf(fmt, args);
	va_end(args);
}