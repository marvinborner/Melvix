// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef MEM_H
#define MEM_H

#include <def.h>

void *malloc_debug(u32 size, const char *file, int line, const char *func, const char *inp);
void free_debug(void *ptr, const char *file, int line, const char *func, const char *inp);
#define malloc(size) malloc_debug(size, __FILE__, __LINE__, __func__, #size)
#define free(ptr) free_debug(ptr, __FILE__, __LINE__, __func__, #ptr)

/* void *_malloc(u32 size); */
/* void _free(void *ptr); */
/* #define malloc(size) _malloc(size) */
/* #define free(ptr) _free(ptr) */

#ifdef kernel
void heap_init(u32 start);
#elif defined(userspace)
#else
#error "No lib target specified. Please use -Dkernel or -Duserspace"
#endif

void *memcpy(void *dest, const void *src, u32 n);
void *memset(void *dest, int val, u32 n);
void *memchr(void *src, int c, u32 n);
int memcmp(const void *s1, const void *s2, u32 n);

#endif
