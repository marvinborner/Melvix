#include <io/io.h>
#include <lib/stdlib.h>
#include <lib/string.h>
#include <memory/alloc.h>
#include <stdarg.h>
#include <stdint.h>

void serial_print(const char *data)
{
	for (u32 i = 0; i < strlen(data); i++)
		serial_put(data[i]);
}

void serial_vprintf(const char *fmt, va_list args)
{
	u8 readyToFormat = 0;

	char buff = 0;

	for (; *fmt; fmt++) {
		if (readyToFormat) {
			if (*fmt == '%') {
				serial_put('%');
				readyToFormat = 0;
				continue;
			}

			buff = *fmt;
			if (buff == 's') {
				const char *str = va_arg(args, const char *);
				serial_print(str);
				readyToFormat = 0;
			} else if (buff == 'x') {
				char *p = htoa((u32)va_arg(args, int));
				serial_print(p);
				free(p);
				readyToFormat = 0;
			} else if (buff == 'd') {
				char *p = itoa(va_arg(args, int));
				serial_print(p);
				free(p);
				readyToFormat = 0;
			} else if (buff == 'c') {
				serial_put((char)va_arg(args, int));
				readyToFormat = 0;
			}
		} else {
			if (*fmt == '%')
				readyToFormat = 1;
			else
				serial_put(*fmt);
		}
	}
}

void serial_printf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	serial_vprintf(fmt, args);
	va_end(args);
}