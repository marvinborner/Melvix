// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <mem.h>
#include <random.h>

static u32 g_seed = 0;

void srand(u32 seed)
{
	g_seed = seed;
}

u32 rand()
{
	g_seed = (214013 * g_seed + 2531011);
	return (g_seed >> 16) & 0x7FFF;
}

char *randstr(u32 size)
{
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
