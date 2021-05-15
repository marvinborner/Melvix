// MIT License, Copyright (c) 2020 Marvin Borner
// Most net/ip handlers are in the kernel space
// This is a userspace wrapper for some things

#include <def.h>
#include <libnet/net.h>
#include <mem.h>
#include <str.h>

// Inspired by Paul Vixie, 1996
int ip_pton(const char *src, u32 *dst)
{
	const char *end = src + strlen(src);
	u8 tmp[4], *tp;
	int saw_digit = 0;
	int octets = 0;
	*(tp = tmp) = 0;

	while (src < end) {
		int ch = *src++;
		if (ch >= '0' && ch <= '9') {
			u32 new = *tp * 10 + (ch - '0');

			if ((saw_digit && *tp == 0) || new > 255)
				return 0;

			*tp = new;
			if (!saw_digit) {
				if (++octets > 4)
					return 0;
				saw_digit = 1;
			}
		} else if (ch == '.' && saw_digit) {
			if (octets == 4)
				return 0;
			*++tp = 0;
			saw_digit = 0;
		} else {
			return 0;
		}
	}

	if (octets < 4)
		return 0;

	*dst = htonl(*(u32 *)tmp);
	return 1;
}
