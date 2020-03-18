#include <stdarg.h>
#include <stdint.h>
#include <kernel/lib/string.h>
#include <kernel/lib/stdlib.h>
#include <kernel/io/io.h>
#include <kernel/memory/alloc.h>

void _write_serial(const char *data)
{
	for (size_t i = 0; i < strlen(data); i++)
		serial_put(data[i]);
}

void serial_printf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

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
				_write_serial(str);
				readyToFormat = 0;
			} else if (buff == 'x') {
				char *p = htoa((uint32_t)va_arg(args, int));
				_write_serial(p);
				kfree(p);
				readyToFormat = 0;
			} else if (buff == 'd') {
				char *p = itoa(va_arg(args, int));
				_write_serial(p);
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

	serial_put('\n');

	va_end(args);
}