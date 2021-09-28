// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef MEM_H
#define MEM_H

#include <def.h>

/**
 * malloc
 */

ATTR((malloc)) ATTR((alloc_size(1))) RET_NONNULL void *_malloc(u32 size);

ATTR((malloc))
ATTR((alloc_size(1)))
RET_NONNULL void *malloc_debug(u32 size, const char *file, int line, const char *func,
			       const char *inp) NONNULL;

/**
 * realloc
 */

ATTR((malloc)) ATTR((alloc_size(2))) void *_realloc(void *ptr, u32 size);

ATTR((malloc))
ATTR((alloc_size(2)))
void *realloc_debug(void *ptr, u32 size, const char *file, int line, const char *func,
		    const char *inp0, const char *inp1);

/**
 * zalloc
 */

ATTR((malloc)) ATTR((alloc_size(1))) RET_NONNULL void *_zalloc(u32 size);

ATTR((malloc))
ATTR((alloc_size(1)))
RET_NONNULL void *zalloc_debug(u32 size, const char *file, int line, const char *func,
			       const char *inp) NONNULL;

/**
 * free
 */

void _free(void *ptr) NONNULL;
void free_debug(void *ptr, const char *file, int line, const char *func, const char *inp) NONNULL;

/**
 * Debug wrappers
 */

#if DEBUG_ALLOC
#define realloc(ptr, size)                                                                         \
	realloc_debug((void *)ptr, (u32)(size), __FILE__, __LINE__, __func__, #ptr, #size)
#define zalloc(size) zalloc_debug((u32)(size), __FILE__, __LINE__, __func__, #size)
#define malloc(size) malloc_debug((u32)(size), __FILE__, __LINE__, __func__, #size)
#define free(ptr) free_debug((void *)(ptr), __FILE__, __LINE__, __func__, #ptr)
#else
#define realloc(ptr, size) _realloc((void *)ptr, (u32)(size))
#define zalloc(size) _zalloc((u32)(size))
#define malloc(size) _malloc((u32)(size))
#define free(ptr) _free((void *)(ptr))
#endif

/**
 * Standard memory functions
 */

void *memcpy(void *dest, const void *src, u32 n) NONNULL;
void *memset(void *dest, u32 val, u32 n) NONNULL;
const void *memcchr(const void *src, char c, u32 n) NONNULL;
void *memchr(void *src, char c, u32 n) NONNULL;
s32 memcmp(const void *s1, const void *s2, u32 n) NONNULL;
u8 mememp(const u8 *buf, u32 n) NONNULL;

#endif
