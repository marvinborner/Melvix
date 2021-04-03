// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <mem.h>
#include <random.h>

#ifdef KERNEL
#include <cpu.h>
#endif

static u32 g_seed = 1;

void srand(u32 seed)
{
	g_seed = seed;
}

static u32 default_rand(void)
{
	g_seed = g_seed * 1103515245 + 12345;
	return (g_seed >> 16) & 0x7FFF;
}

u32 rand(void)
{
#ifdef KERNEL
	u32 rd;
	if (cpu_extended_features.ebx & CPUID_EXT_FEAT_EBX_RDSEED) {
		__asm__ volatile("1:\n"
				 "rdseed %0\n"
				 "jnc 1b\n"
				 : "=r"(rd));
	} else if (cpu_features.ecx & CPUID_FEAT_ECX_RDRND) {
		__asm__ volatile("1:\n"
				 "rdrand %0\n"
				 "jnc 1b\n"
				 : "=r"(rd));
	} else {
		rd = default_rand();
	}

	return rd;
#else
	return default_rand();
#endif
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
