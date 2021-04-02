// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef MEM_H
#define MEM_H

#include <def.h>

void *malloc_debug(u32 size, const char *file, int line, const char *func, const char *inp) NONNULL;
void free_debug(void *ptr, const char *file, int line, const char *func, const char *inp) NONNULL;
#define malloc(size) malloc_debug((u32)(size), __FILE__, __LINE__, __func__, #size)
#define free(ptr) free_debug((void *)(ptr), __FILE__, __LINE__, __func__, #ptr)
void *realloc(void *ptr, u32 size);
void *zalloc(u32 size);

#ifdef KERNEL
#define STACK_START 0x00500000 // Defined it bootloader
#define STACK_SIZE 0x1000 // idk
#elif defined(USER)
#endif

void *memcpy(void *dest, const void *src, u32 n) NONNULL;
void *memset(void *dest, u32 val, u32 n) NONNULL;
void *memchr(void *src, char c, u32 n) NONNULL;
int memcmp(const void *s1, const void *s2, u32 n) NONNULL;
int mememp(const u8 *buf, u32 n) NONNULL;

#endif
