// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <def.h>
#include <mem.h>
#include <sys.h>

void *memcpy(void *dest, const void *src, u32 n)
{
#ifdef USER
	// Inspired by Jeko at osdev
	u8 *dest_byte = dest;
	const u8 *src_byte = src;
	for (u32 i = 0; i < n / 16; i++) {
		__asm__ volatile("movups (%0), %%xmm0\n"
				 "movntdq %%xmm0, (%1)\n" ::"r"(src_byte),
				 "r"(dest_byte)
				 : "memory");

		src_byte += 16;
		dest_byte += 16;
	}

	if (n & 7) {
		n = n & 7;

		int d0, d1, d2;
		__asm__ volatile("rep ; movsl\n\t"
				 "testb $2,%b4\n\t"
				 "je 1f\n\t"
				 "movsw\n"
				 "1:\ttestb $1,%b4\n\t"
				 "je 2f\n\t"
				 "movsb\n"
				 "2:"
				 : "=&c"(d0), "=&D"(d1), "=&S"(d2)
				 : "0"(n / 4), "q"(n), "1"((long)dest_byte), "2"((long)src_byte)
				 : "memory");
	}
	return dest_byte;
#else
	// Inspired by jgraef at osdev
	u32 num_dwords = n / 4;
	u32 num_bytes = n % 4;
	u32 *dest32 = (u32 *)dest;
	const u32 *src32 = (const u32 *)src;
	u8 *dest8 = ((u8 *)dest) + num_dwords * 4;
	const u8 *src8 = ((const u8 *)src) + num_dwords * 4;

	// TODO: What's faster?
	__asm__ volatile("rep movsl\n"
			 : "=S"(src32), "=D"(dest32), "=c"(num_dwords)
			 : "S"(src32), "D"(dest32), "c"(num_dwords)
			 : "memory");

	/* for (u32 i = 0; i < num_dwords; i++) { */
	/* 	dest32[i] = src32[i]; */
	/* } */

	for (u32 i = 0; i < num_bytes; i++) {
		dest8[i] = src8[i];
	}
	return dest;
#endif
}

void *memset(void *dest, u32 val, u32 n)
{
	u32 uval = val;
	u32 num_dwords = n / 4;
	u32 num_bytes = n % 4;
	u32 *dest32 = (u32 *)dest;
	u8 *dest8 = ((u8 *)dest) + num_dwords * 4;
	u8 val8 = (u8)val;
	u32 val32 = uval | (uval << 8) | (uval << 16) | (uval << 24);

	// TODO: What's faster?
	__asm__ volatile("rep stosl\n"
			 : "=D"(dest32), "=c"(num_dwords)
			 : "D"(dest32), "c"(num_dwords), "a"(val32)
			 : "memory");

	/* for (u32 i = 0; i < num_dwords; i++) { */
	/* 	dest32[i] = val32; */
	/* } */

	for (u32 i = 0; i < num_bytes; i++) {
		dest8[i] = val8;
	}
	return dest;
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

#ifdef KERNEL

#include <cpu.h>

void *memcpy_user(void *dest, const void *src, u32 n)
{
	stac();
	void *ret = memcpy(dest, src, n);
	clac();
	return ret;
}

void *memset_user(void *dest, u32 val, u32 n)
{
	stac();
	void *ret = memset(dest, val, n);
	clac();
	return ret;
}

void *memchr_user(void *src, char c, u32 n)
{
	stac();
	void *ret = memchr(src, c, n);
	clac();
	return ret;
}

s32 memcmp_user(const void *s1, const void *s2, u32 n)
{
	stac();
	s32 ret = memcmp(s1, s2, n);
	clac();
	return ret;
}

#endif
