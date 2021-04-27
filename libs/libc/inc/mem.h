// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef MEM_H
#define MEM_H

#include <def.h>

ATTR((malloc))
ATTR((alloc_size(1)))
RET_NONNULL void *malloc_debug(u32 size, const char *file, int line, const char *func,
			       const char *inp) NONNULL;
ATTR((malloc)) ATTR((alloc_size(2))) RET_NONNULL void *realloc(void *ptr, u32 size);
ATTR((malloc)) ATTR((alloc_size(1))) RET_NONNULL void *zalloc(u32 size);
void free_debug(void *ptr, const char *file, int line, const char *func, const char *inp) NONNULL;

#define malloc(size) malloc_debug((u32)(size), __FILE__, __LINE__, __func__, #size)
#define free(ptr) free_debug((void *)(ptr), __FILE__, __LINE__, __func__, #ptr)

#ifdef KERNEL
#define STACK_START 0x00500000 // Defined in bootloader
#define STACK_SIZE (1 << 20) // 1MiB
#elif defined(USER)
#endif

void *memcpy(void *dest, const void *src, u32 n) NONNULL;
void *memset(void *dest, u32 val, u32 n) NONNULL;
void *memchr(void *src, char c, u32 n) NONNULL;
s32 memcmp(const void *s1, const void *s2, u32 n) NONNULL;
u8 mememp(const u8 *buf, u32 n) NONNULL;

#ifdef KERNEL
void *memcpy_user(void *dest, const void *src, u32 n) NONNULL;
void *memset_user(void *dest, u32 val, u32 n) NONNULL;
void *memchr_user(void *src, char c, u32 n) NONNULL;
s32 memcmp_user(const void *s1, const void *s2, u32 n) NONNULL;
u8 mememp_user(const u8 *buf, u32 n) NONNULL;
#endif

#endif
