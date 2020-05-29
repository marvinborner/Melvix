#include <lib/stdlib.h>
#include <stdarg.h>
#include <stdint.h>

static void append(char *dest, char *src, int index)
{
	for (int i = index; i < strlen(src) + index; i++)
		dest[i] = src[i - index];
	dest[index + strlen(src)] = 0;
}

int vsprintf(char *str, const char *fmt, va_list args)
{
	u8 ready_to_format = 0;

	int i = 0;
	char buf = 0;
	char format_buffer[20] = "\0";

	for (; *fmt; fmt++) {
		if (ready_to_format) {
			ready_to_format = 0;

			if (*fmt == '%') {
				str[i] = '%';
				continue;
			}

			buf = *fmt;

			// TODO: Improve this repetitive code
			if (buf == 's') {
				char *string = va_arg(args, char *);
				append(str, string, i);
				i = strlen(str);
			} else if (buf == 'x') {
				itoa_base(va_arg(args, u32), format_buffer, 16, 0);
				append(str, format_buffer, i);
				i = strlen(str);
			} else if (buf == 'd' || buf == 'i') {
				itoa_base(va_arg(args, s32), format_buffer, 10, 1);
				append(str, format_buffer, i);
				i = strlen(str);
			} else if (buf == 'u') {
				itoa_base(va_arg(args, u32), format_buffer, 10, 0);
				append(str, format_buffer, i);
				i = strlen(str);
			} else if (buf == 'o') {
				itoa_base(va_arg(args, u32), format_buffer, 8, 0);
				append(str, format_buffer, i);
				i = strlen(str);
			} else if (buf == 'b') {
				itoa_base(va_arg(args, u32), format_buffer, 2, 0);
				append(str, format_buffer, i);
				i = strlen(str);
			} else if (buf == 'c') {
				str[i] = (char)va_arg(args, int);
				i++;
			}
		} else {
			if (*fmt == '%')
				ready_to_format = 1;
			else {
				str[i] = *fmt;
				i++;
			}
		}

		format_buffer[0] = '\0';
	}

	return strlen(str);
}