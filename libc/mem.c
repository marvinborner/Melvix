// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <print.h>
#include <sys.h>

// Taken from jgraef at osdev
void *memcpy(void *dest, const void *src, u32 n)
{
	u32 num_dwords = n / 4;
	u32 num_bytes = n % 4;
	u32 *dest32 = (u32 *)dest;
	u32 *src32 = (u32 *)src;
	u8 *dest8 = ((u8 *)dest) + num_dwords * 4;
	u8 *src8 = ((u8 *)src) + num_dwords * 4;
	u32 i;

	for (i = 0; i < num_dwords; i++) {
		dest32[i] = src32[i];
	}
	for (i = 0; i < num_bytes; i++) {
		dest8[i] = src8[i];
	}
	return dest;
}

void *memset(void *dest, char val, u32 n)
{
	u32 num_dwords = n / 4;
	u32 num_bytes = n % 4;
	u32 *dest32 = (u32 *)dest;
	u8 *dest8 = ((u8 *)dest) + num_dwords * 4;
	u8 val8 = (u8)val;
	u32 val32 = val | (val << 8) | (val << 16) | (val << 24);
	u32 i;

	for (i = 0; i < num_dwords; i++) {
		dest32[i] = val32;
	}
	for (i = 0; i < num_bytes; i++) {
		dest8[i] = val8;
	}
	return dest;
}

int memcmp(const void *s1, const void *s2, u32 n)
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
