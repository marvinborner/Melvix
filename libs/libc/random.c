// MIT License, Copyright (c) 2020 Marvin Borner

#include <cpu.h>
#include <def.h>
#include <mem.h>
#include <random.h>

static u32 g_seed = 1;

void srand(u32 seed)
{
	g_seed = seed;
}

u32 rdrand(void)
{
#ifdef KERNEL
	if (!cpu_has_cfeature(CPUID_FEAT_ECX_RDRND))
		return rand();

	u32 rd;
	__asm__ volatile("1:\n"
			 "rdrand %0\n"
			 "jnc 1b\n"
			 : "=r"(rd));
	return rd;
#else
	return rand();
#endif
}

u32 rdseed(void)
{
#ifdef KERNEL
	if (!cpu_has_cfeature(CPUID_FEAT_ECX_RDRND))
		return rand();

	u32 rd;
	__asm__ volatile("1:\n"
			 "rdseed %0\n"
			 "jnc 1b\n"
			 : "=r"(rd));
	return rd;
#else
	return rand();
#endif
}

u32 rand(void)
{
	g_seed = g_seed * 1103515245 + 12345;
	return (g_seed >> 16) & 0x7FFF;
}

char *randstr(u32 size)
{
	if (!size)
		return NULL;

	char *buf = malloc(size + 1);
	const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

	size--;
	for (u32 i = 0; i < size; i++) {
		int key = rand() % (sizeof(charset) - 1);
		buf[i] = charset[key];
	}
	buf[size] = '\0';

	return buf;
}
