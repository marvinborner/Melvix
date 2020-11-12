// MIT License, Copyright (c) 2020 Marvin Borner

#include <arg.h>
#include <conv.h>
#include <def.h>
#include <mem.h>
#include <serial.h>
#include <str.h>

static void append(char *dest, char *src, int index)
{
	for (u32 i = index; i < strlen(src) + index; i++)
		dest[i] = src[i - index];
	dest[index + strlen(src)] = 0;
}

int vsprintf(char *str, const char *format, va_list ap)
{
	u8 ready_to_format = 0;

	int i = 0;
	char buf = 0;
	char format_buffer[20] = { '\0' };

	for (; *format; format++) {
		if (ready_to_format) {
			ready_to_format = 0;

			if (*format == '%') {
				str[i] = '%';
				continue;
			}

			buf = *format;

			// TODO: Improve this repetitive code
			if (buf == 's') {
				char *string = va_arg(ap, char *);
				append(str, string, i);
				i = strlen(str);
			} else if (buf == 'x') {
				conv_base(va_arg(ap, u32), format_buffer, 16, 0);
				append(str, format_buffer, i);
				i = strlen(str);
			} else if (buf == 'd' || buf == 'i') {
				conv_base(va_arg(ap, s32), format_buffer, 10, 1);
				append(str, format_buffer, i);
				i = strlen(str);
			} else if (buf == 'u') {
				conv_base(va_arg(ap, u32), format_buffer, 10, 0);
				append(str, format_buffer, i);
				i = strlen(str);
			} else if (buf == 'o') {
				conv_base(va_arg(ap, u32), format_buffer, 8, 0);
				append(str, format_buffer, i);
				i = strlen(str);
			} else if (buf == 'b') {
				conv_base(va_arg(ap, u32), format_buffer, 2, 0);
				append(str, format_buffer, i);
				i = strlen(str);
			} else if (buf == 'c') {
				str[i] = (char)va_arg(ap, int);
				i++;
			}
		} else {
			if (*format == '%')
				ready_to_format = 1;
			else {
				str[i] = *format;
				i++;
			}
		}

		format_buffer[0] = '\0';
	}

	return strlen(str);
}

int vprintf(const char *format, va_list ap)
{
	char buf[1024] = { 0 };
	int len = vsprintf(buf, format, ap);
	serial_print(buf); // TODO: Remove temporary serial print
	return len;
}

// TODO: Fix printf for *very* large strings (serial works)
int printf(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int len = vprintf(format, ap);
	va_end(ap);

	return len;
}

int print(const char *str)
{
	serial_print(str);
	return strlen(str);
}
