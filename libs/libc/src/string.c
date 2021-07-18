// MIT License, Copyright(c) 2021 Marvin Borner

#include <stdint.h>
#include <string.h>

/**
 * String functions
 */

size_t strlen(const char *s)
{
	const char *str = s;
	while (*str)
		str++;

	return str - s;
}

size_t strnlen(const char *s, size_t maxlen)
{
	const char *str = s;
	while (maxlen && *str) {
		str++;
		maxlen--;
	}

	return str - s;
}

size_t strlcpy(char *dst, const char *src, size_t size)
{
	const char *orig = src;
	size_t left = size;

	if (left)
		while (--left)
			if (!(*dst++ = *src++))
				break;

	if (!left) {
		if (size)
			*dst = 0;
		while (*src++)
			;
	}

	return src - orig - 1;
}

size_t strlcat(char *dst, const char *src, size_t size)
{
	const char *orig_dst = dst;
	const char *orig_src = src;

	size_t n = size;
	while (n-- && *dst)
		dst++;

	size_t len = dst - orig_dst;
	n = size - len;

	if (!n--)
		return len + strlen(src);

	while (*src) {
		if (n) {
			*dst++ = *src;
			n--;
		}
		src++;
	}
	*dst = 0;

	return len + (src - orig_src);
}

int strcmp(const char *s1, const char *s2)
{
	const char *c1 = (const char *)s1;
	const char *c2 = (const char *)s2;
	char ch;
	int d = 0;

	while (1) {
		d = (int)(ch = *c1++) - (int)*c2++;
		if (d || !ch)
			break;
	}

	return d;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	const char *c1 = (const char *)s1;
	const char *c2 = (const char *)s2;
	char ch;
	int d = 0;

	while (n--) {
		d = (int)(ch = *c1++) - (int)*c2++;
		if (d || !ch)
			break;
	}

	return d;
}

/**
 * Memory functions
 */

void *memcpy(void *dst, const void *src, size_t n)
{
	// Inspired by jgraef at osdev
	size_t num_dwords = n / 4;
	size_t num_bytes = n % 4;

	uint32_t *dst32 = (uint32_t *)dst;
	const uint32_t *src32 = (const uint32_t *)src;

	uint8_t *dst8 = ((uint8_t *)dst) + num_dwords * 4;
	const uint8_t *src8 = ((const uint8_t *)src) + num_dwords * 4;

	__asm__ volatile("rep movsl\n"
			 : "=S"(src32), "=D"(dst32), "=c"(num_dwords)
			 : "S"(src32), "D"(dst32), "c"(num_dwords)
			 : "memory");

	for (size_t i = 0; i < num_bytes; i++)
		dst8[i] = src8[i];

	return dst;
}

void *memset(void *dst, int c, size_t n)
{
	// Inspired by jgraef at osdev
	uint32_t uval = c; // TODO: 64-Bit compat?
	size_t num_dwords = n / 4;
	size_t num_bytes = n % 4;

	uint32_t *dst32 = (uint32_t *)dst;
	uint32_t val32 = uval | (uval << 8) | (uval << 16) | (uval << 24);

	uint8_t *dst8 = ((uint8_t *)dst) + num_dwords * 4;
	uint8_t val8 = (uint8_t)c;

	__asm__ volatile("rep stosl\n"
			 : "=D"(dst32), "=c"(num_dwords)
			 : "D"(dst32), "c"(num_dwords), "a"(val32)
			 : "memory");

	for (size_t i = 0; i < num_bytes; i++)
		dst8[i] = val8;

	return dst;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
	const char *a = (const char *)s1;
	const char *b = (const char *)s2;
	for (size_t i = 0; i < n; i++) {
		if (a[i] < b[i])
			return -1;
		else if (b[i] < a[i])
			return 1;
	}
	return 0;
}
