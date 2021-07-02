// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef UTF8_H
#define UTF8_H

#include <def.h>

static void *utf8_decode(void *buf, u32 *c, u32 *e)
{
	static const u8 lengths[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				      0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0 };
	static const u8 masks[] = { 0x00, 0x7f, 0x1f, 0x0f, 0x07 };
	static const u32 mins[] = { 4194304, 0, 128, 2048, 65536 };
	static const u8 shiftc[] = { 0, 18, 12, 6, 0 };
	static const u8 shifte[] = { 0, 6, 4, 2, 0 };

	u8 *s = buf;
	u8 len = lengths[s[0] >> 3];

	u8 *next = s + len + !len;

	*c = (u32)(s[0] & masks[len]) << 18;
	*c |= (u32)(s[1] & 0x3f) << 12;
	*c |= (u32)(s[2] & 0x3f) << 6;
	*c |= (u32)(s[3] & 0x3f) << 0;
	*c >>= shiftc[len];

	*e = (*c < mins[len]) << 6;
	*e |= ((*c >> 11) == 0x1b) << 7;
	*e |= (*c > 0x10FFFF) << 8;
	*e |= (s[1] & 0xc0) >> 2;
	*e |= (s[2] & 0xc0) >> 4;
	*e |= (s[3]) >> 6;
	*e ^= 0x2a;
	*e >>= shifte[len];

	return next;
}

#endif
