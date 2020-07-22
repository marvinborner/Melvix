// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef DEF_H
#define DEF_H

/**
 * Types
 */

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed long s32;
typedef unsigned long u32;

typedef signed long long s64;
typedef unsigned long long u64;

/**
 * Macros
 */

#define NULL ((void *)0)
#define malloc(n) ((void *)((HEAP += n) - n)) // TODO: Implement real/better malloc/free
#define free(x)

/**
 * Heap
 */

extern u32 HEAP;
extern u32 HEAP_START;

#endif
