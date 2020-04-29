#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void vprintf(char *fmt, va_list args)
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
				char *str = va_arg(args, char *);
				puts(str);
				readyToFormat = 0;
			} else if (buff == 'x') {
				char *p = htoa((u32)va_arg(args, int));
				puts(p);
				free(p);
				readyToFormat = 0;
			} else if (buff == 'd') {
				char *p = itoa(va_arg(args, int));
				puts(p);
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