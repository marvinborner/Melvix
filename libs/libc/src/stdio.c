// MIT License, Copyright(c) 2021 Marvin Borner

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
	int length = 0;

	int temp_int;
	char temp_ch;
	char *temp_str;

	char buffer[64] = { 0 };

	// TODO: Fix potential memory overflows because of str[length++]=xxx
	char ch;
	while ((ch = *format++)) {
		if (ch == '%') {
			switch (*format++) {
			case '%':
				str[length++] = '%';
				break;
			case 'c':
				temp_ch = va_arg(ap, int);
				str[length++] = temp_ch;
				break;
			case 's':
				temp_str = va_arg(ap, char *);
				length += strlcpy(&str[length], temp_str, size - length);
				break;
			case 'b':
				temp_int = va_arg(ap, int);
				itoa(temp_int, buffer, 2);
				length += strlcpy(&str[length], buffer, size - length);
				break;
			case 'd':
				temp_int = va_arg(ap, int);
				itoa(temp_int, buffer, 10);
				length += strlcpy(&str[length], buffer, size - length);
				break;
			case 'x':
				temp_int = va_arg(ap, int);
				itoa(temp_int, buffer, 16);
				length += strlcpy(&str[length], buffer, size - length);
				break;
			default:
				// Unknown printf format?
				break;
			}
		} else {
			str[length++] = ch;
		}
	}

	return length;
}

int snprintf(char *str, size_t size, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int len = vsnprintf(str, size, format, ap);
	va_end(ap);

	return len;
}

int vprintf(const char *format, va_list ap)
{
	return vdprintf(DEV_LOGGER, format, ap);
}

int vfprintf(const char *path, const char *format, va_list ap)
{
	char buf[1024] = { 0 };
	int len = vsnprintf(buf, sizeof(buf), format, ap);
	return fs_write(path, buf, 0, len);
}

int vdprintf(dev_t type, const char *format, va_list ap)
{
	char buf[1024] = { 0 };
	int len = vsnprintf(buf, sizeof(buf), format, ap);
	return dev_write(type, buf, 0, len);
}

int fprintf(const char *path, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int len = vfprintf(path, format, ap);
	va_end(ap);

	return len;
}

int dprintf(dev_t type, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int len = vdprintf(type, format, ap);
	va_end(ap);

	return len;
}

int printf(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int len = vprintf(format, ap);
	va_end(ap);

	return len;
}
