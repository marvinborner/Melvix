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

typedef float f32;
typedef double f64;
typedef long double f80;

/**
 * Useful macros
 */

#define UNUSED(a) ((void)(a))

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define ABS(a) ((u32)(((s32)(a) < 0) ? -(a) : (a)))
#define FABS(a) ((f64)(((f64)(a) < 0.0) ? -(a) : (a)))

#define COUNT(a) (sizeof(a) / sizeof 0 [a])

#define UPPER(a) ((a) >= 'A' && ((a) <= 'Z'))
#define LOWER(a) ((a) >= 'a' && ((a) <= 'z'))
#define ALPHA(a) (UPPER(a) || LOWER(a))
#define NUMERIC(a) ((a) >= '0' && ((a) <= '9'))
#define ALPHANUMERIC(a) (ALPHA(a) || NUMERIC(a))

#define STRINGIFY_PARAM(a) #a
#define STRINGIFY(a) STRINGIFY_PARAM(a)

#define ALIGN_UP(addr, align) (((addr) + (align)-1) & ~((align)-1))
#define ALIGN_DOWN(addr, align) ((addr) & ~((align)-1))

/**
 * Compiler attribute wrappers
 */

#define ATTR __attribute__
#define NORETURN ATTR((noreturn))
#define INLINE ATTR((gnu_inline)) inline
#define NOINLINE ATTR((noinline))
#define DEPRECATED ATTR((deprecated))
#define NONNULL ATTR((nonnull))
#define RET_NONNULL ATTR((returns_nonnull))
#define PURE ATTR((pure))
#define CONST ATTR((const))
#define FLATTEN ATTR((flatten))
#define PACKED ATTR((packed))
#define HOT ATTR((hot))
#define SENTINEL ATTR((sentinel))
#define USED_FUNC ATTR((used))
#define UNUSED_FUNC ATTR((unused))
#define NO_SANITIZE ATTR((no_sanitize("undefined")))
#define OPTIMIZE(level) ATTR((optimize(level)))
#define ALIGNED(align) ATTR((aligned(align)))

/**
 * Kernel section macros
 */

#define TEMPORARY NOINLINE ATTR((section(".temp_clear")))
#define PROTECTED ATTR((section(".temp_protect")))

/**
 * General macro numbers
 */

#define EOF (-1)
#define NULL ((void *)0)

#define U8_MAX ((u8)255)
#define S8_MAX ((s8)127)
#define S8_MIN ((s8)-128)
#define U16_MAX ((u16)65535)
#define S16_MAX ((s16)32767)
#define S16_MIN ((s16)-32768)
#define U32_MAX ((u32)4294967295)
#define S32_MAX ((s32)2147483647)
#define S32_MIN ((s32)-2147483648)
#define F64_MAX ((f64)0x7fefffffffffffff)
#define F64_MIN ((f64)0x10000000000000)

#define MILLION 1000000
#define BILLION 1000000000
#define TRILLION 1000000000000
#define QUADRILLION 1000000000000000

#endif
