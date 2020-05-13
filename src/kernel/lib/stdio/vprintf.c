#include <lib/stdio.h>
#include <lib/stdlib.h>
#include <lib/string.h>
#include <memory/alloc.h>
#include <stdarg.h>
#include <stdint.h>

void _puts(const char *data)
{
	for (u32 i = 0; i < strlen(data); i++)
		putch(data[i]);
}

void vprintf(const char *fmt, va_list args)
{
	u8 readyToFormat = 0;

	char buff = 0;

	for (; *fmt; fmt++) {
		if (readyToFormat) {
			if (*fmt == '%') {
				putch('%');
				readyToFormat = 0;
				continue;
			}

			buff = *fmt;
			if (buff == 's') {
				const char *str = va_arg(args, const char *);
				_puts(str);
				readyToFormat = 0;
			} else if (buff == 'x') {
				char *p = htoa((u32)va_arg(args, int));
				_puts(p);
				free(p);
				readyToFormat = 0;
			} else if (buff == 'd') {
				char *p = itoa(va_arg(args, int));
				_puts(p);
				free(p);
				readyToFormat = 0;
			} else if (buff == 'c') {
				putch((char)va_arg(args, int));
				readyToFormat = 0;
			}
		} else {
			if (*fmt == '%')
				readyToFormat = 1;
			else
				putch(*fmt);
		}
	}
}