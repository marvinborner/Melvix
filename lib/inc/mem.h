// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef MEM_H
#define MEM_H

#include <def.h>

u32 HEAP;
u32 HEAP_START;

#define malloc(n) ((void *)((HEAP += n) - n)) // TODO: Implement real/better malloc/free
#define free(x)

void *memcpy(void *dst, const void *src, u32 n);
void *memset(void *dst, int c, u32 n);
int memcmp(const void *s1, const void *s2, u32 n);

#endif
