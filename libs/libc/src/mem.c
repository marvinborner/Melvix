// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <def.h>
#include <mem.h>
#include <sys.h>

void *memcpy(void *dest, const void *src, u32 n)
{
	// Inspired by jgraef at osdev
	u32 num_dwords = n / 4;
	u32 num_bytes = n % 4;
	u32 *dest32 = (u32 *)dest;
	const u32 *src32 = (const u32 *)src;
	u8 *dest8 = ((u8 *)dest) + num_dwords * 4;
	const u8 *src8 = ((const u8 *)src) + num_dwords * 4;

	__asm__ volatile("rep movsl\n"
			 : "=S"(src32), "=D"(dest32), "=c"(num_dwords)
			 : "S"(src32), "D"(dest32), "c"(num_dwords)
			 : "memory");

	for (u32 i = 0; i < num_bytes; i++)
		dest8[i] = src8[i];

	return dest;
}

void *memset(void *dest, u32 val, u32 n)
{
	// Inspired by jgraef at osdev
	u32 uval = val;
	u32 num_dwords = n / 4;
	u32 num_bytes = n % 4;
	u32 *dest32 = (u32 *)dest;
	u8 *dest8 = ((u8 *)dest) + num_dwords * 4;
	u8 val8 = (u8)val;
	u32 val32 = uval | (uval << 8) | (uval << 16) | (uval << 24);

	__asm__ volatile("rep stosl\n"
			 : "=D"(dest32), "=c"(num_dwords)
			 : "D"(dest32), "c"(num_dwords), "a"(val32)
			 : "memory");

	for (u32 i = 0; i < num_bytes; i++)
		dest8[i] = val8;

	return dest;
}

const void *memcchr(const void *src, char c, u32 n)
{
	const u8 *s = (const u8 *)src;

	while (n-- > 0) {
		if (*s == c)
			return s;
		s++;
	}
	return NULL;
}

void *memchr(void *src, char c, u32 n)
{
	u8 *s = (u8 *)src;

	while (n-- > 0) {
		if (*s == c)
			return s;
		s++;
	}
	return NULL;
}

s32 memcmp(const void *s1, const void *s2, u32 n)
{
	const u8 *a = (const u8 *)s1;
	const u8 *b = (const u8 *)s2;
	for (u32 i = 0; i < n; i++) {
		if (a[i] < b[i])
			return -1;
		else if (b[i] < a[i])
			return 1;
	}
	return 0;
}

u8 mememp(const u8 *buf, u32 n)
{
	return buf[0] == 0 && !memcmp(buf, buf + 1, n - 1);
}
