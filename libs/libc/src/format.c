/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <err.h>
#include <format.h>
#include <print.h>

NONNULL static u32 format_int(char *buf, u32 size, s32 integer, u32 base, u8 is_signed)
{
	if (!size)
		return 0;

	u8 sign = base == 10 && integer < 0;
	u32 unsigned_integer;
	if (is_signed && sign)
		unsigned_integer = -integer;
	else
		unsigned_integer = integer;

	char tmp[16];
	char *ptr = tmp;
	while (unsigned_integer || ptr == tmp) {
		int i = unsigned_integer % base;
		unsigned_integer /= base;
		if (i < 10)
			*ptr++ = i + '0';
		else
			*ptr++ = i + 'a' - 10;
	}

	u32 len = ptr - tmp;

	// TODO: Fix potential memory overflows by actually using `size`
	if (is_signed && sign) {
		*buf++ = '-';
		len++;
	}

	while (ptr > tmp)
		*buf++ = *--ptr;
	*buf = '\0';

	return len;
}

u32 format(char *out, u32 size, const char *fmt, va_list ap)
{
	char **buf = &out;
	u32 length = size;

	union {
		char c;
		const char *s;
		s32 i;
		u32 u;
	} u;

	for (; length > 0 && *fmt; fmt++) {
		if (*fmt != '%') {
			**buf = *fmt;
			(*buf)++;
			length--;
			continue;
		}

		switch (*(++fmt)) {
		case 'd':
			u.i = va_arg(ap, s32);
			length -= format_int(*buf, length, u.i, 10, 1);
			break;
		case 'u':
			u.u = va_arg(ap, u32);
			length -= format_int(*buf, length, u.u, 10, 0);
			break;
		case 'x':
			u.u = va_arg(ap, u32);
			length -= format_int(*buf, length, u.u, 16, 0);
			break;
		case 'c':
			u.c = va_arg(ap, int);
			**buf = u.c;
			(*buf)++;
			length--;
			break;
		case 'e':
			u.s = format_error(va_arg(ap, err));
			// TODO: DRY
			while (u.s && *u.s) {
				**buf = *(u.s++);
				(*buf)++;
				length--;
			}
			break;
		case 's':
			u.s = va_arg(ap, const char *);
			while (u.s && *u.s) {
				**buf = *(u.s++);
				(*buf)++;
				length--;
			}
			break;
		case '%':
			**buf = *fmt;
			(*buf)++;
			length--;
			break;
		default:
			panic("Unknown print directive '%%%c'", *fmt);
		}
	}

	return size - length;
}
