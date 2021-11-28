/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

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

typedef float f32;
typedef double f64;
typedef long double f80;

/**
 * Useful macros
 */

#define UNUSED(a) ((void)(a))

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define COUNT(a) (sizeof(a) / sizeof 0 [a])

#define BIT_GET(num, n) (((num) & (1UL << (n))) >> (n))
#define BIT_SET(num, n) ((num) | (1UL << (n)))
#define BIT_CLEAR(num, n) ((num) & ~(1UL << (n)))
#define BIT_TOGGLE(num, n) ((num) ^ (1UL << (n)))

/**
 * Compiler attribute wrappers
 */

#define ATTR __attribute__
#define NORETURN ATTR((noreturn))
#define INLINE ATTR((gnu_inline)) inline
#define NOINLINE ATTR((noinline))
#define DEPRECATED ATTR((deprecated))
#define NONNULL ATTR((nonnull))
#define PACKED ATTR((packed))
#define HOT ATTR((hot))
#define OPTIMIZE(level) ATTR((optimize(level)))
#define ALIGNED(align) ATTR((aligned(align)))

/**
 * Kernel section macros
 */

#define TEMPORARY NOINLINE ATTR((section(".temporary")))
#define PROTECTED ATTR((section(".protected")))

/**
 * General macro constants
 */

#define NULL ((void *)0)

#endif
