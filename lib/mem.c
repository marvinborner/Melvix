// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>

void *memcpy(void *dst, const void *src, u32 n)
{
	const char *sp = (const char *)src;
	char *dp = (char *)dst;
	for (; n != 0; n--)
		*dp++ = *sp++;
	return dst;
}

void *memset(void *dst, char val, u32 n)
{
	char *temp = (char *)dst;
	for (; n != 0; n--)
		*temp++ = val;
	return dst;
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
