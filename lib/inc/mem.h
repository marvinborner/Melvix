// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef MEM_H
#define MEM_H

#include <def.h>

// Huh
#ifdef kernel
#define malloc(n) (void *)((HEAP += n) - n) // TODO: Implement real/better malloc/free
#define free(ptr)
#elif defined(userspace)
#include <sys.h>
#define malloc(n) (void *)sys1(SYS_MALLOC, n)
#define free(ptr) (void)(sys1(SYS_FREE, (int)ptr))
#else
#error "No lib target specified. Please use -Dkernel or -Duserspace"
#endif

u32 HEAP;
u32 HEAP_START;

void *memcpy(void *dst, const void *src, u32 n);
void *memset(void *dst, int c, u32 n);
int memcmp(const void *s1, const void *s2, u32 n);

#endif
