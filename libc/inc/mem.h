// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef MEM_H
#define MEM_H

#include <def.h>

// Huh
#ifdef kernel
#define HEAP_SIZE 0x10000
#define HEAP_MAGIC 0x42
void heap_init(u32 start);
void *malloc(u32 size);
void free(void *ptr);
#elif defined(userspace)
void *malloc(u32 size);
void free(void *ptr);
#else
#error "No lib target specified. Please use -Dkernel or -Duserspace"
#endif

void *memcpy(void *dest, const void *src, u32 n);
void *memset(void *dest, int val, u32 n);
void *memchr(void *src, int c, u32 n);
int memcmp(const void *s1, const void *s2, u32 n);

#endif
