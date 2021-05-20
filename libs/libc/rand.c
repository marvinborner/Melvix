// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <mem.h>
#include <rand.h>

#ifdef KERNEL
#include <drivers/cpu.h>
#endif

static u32 g_seed = 1;

void srand(u32 seed)
{
	g_seed = seed;
}

static u32 rand_default(void)
{
	g_seed = g_seed * 1103515245 + 12345;
	return (g_seed >> 16) & 0x7FFF;
}

static u32 rand_once(void)
{
#ifdef KERNEL
	u32 val = 0;
	if (cpu_extended_features.ebx & CPUID_EXT_FEAT_EBX_RDSEED) {
		__asm__ volatile("1:\n"
				 "rdseed %0\n"
				 "jnc 1b\n"
				 : "=r"(val));
	} else if (cpu_features.ecx & CPUID_FEAT_ECX_RDRND) {
		__asm__ volatile("1:\n"
				 "rdrand %0\n"
				 "jnc 1b\n"
				 : "=r"(val));
	} else {
		val = rand_default();
	}

	return val;
#else
	return rand_default();
#endif
}

u32 rand(void)
{
	u32 ret = 0;
	ret |= (rand_once() & (0xffu << 0));
	ret |= (rand_once() & (0xffu << 8));
	ret |= (rand_once() & (0xffu << 16));
	ret |= (rand_once() & (0xffu << 24));
	return ret;
}

void rand_fill(void *buf, u32 size)
{
	for (u32 i = 0; i < size; i++)
		((u8 *)buf)[i] = rand_once() & 0xff;
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
