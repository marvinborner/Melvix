#include <stdarg.h>
#include <stdint.h>
#include <kernel/lib/string.h>
#include <kernel/lib/stdlib.h>
#include <kernel/io/io.h>
#include <kernel/memory/alloc.h>

void serial_print(const char *data)
{
	for (size_t i = 0; i < strlen(data); i++)
		serial_put(data[i]);
}

void serial_vprintf(const char *fmt, va_list args)
{
	uint8_t readyToFormat = 0;

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
				char *p = htoa((uint32_t)va_arg(args, int));
				serial_print(p);
				kfree(p);
				readyToFormat = 0;
			} else if (buff == 'd') {
				char *p = itoa(va_arg(args, int));
				serial_print(p);
				kfree(p);
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