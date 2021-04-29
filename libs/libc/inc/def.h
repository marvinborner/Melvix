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

#define UNUSED(a) ((void)(a))

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define ABS(a) ((u32)(((s32)(a) < 0) ? -(a) : (a)))

#define __STRINGIFY(a) #a
#define STRINGIFY(a) __STRINGIFY(a)

#define ATTR __attribute__
#define NORETURN ATTR((noreturn))
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
#define ALIGNED(align) ATTR((aligned(align)))

#ifdef KERNEL
#define CLEAR NOINLINE ATTR((section(".temp_clear")))
#define PROTECTED ATTR((section(".temp_protect")))
#endif

#define EOF (-1)
#define NULL ((void *)0)

#define U8_MAX 255
#define S8_MAX 127
#define S8_MIN -128
#define U16_MAX 65535
#define S16_MAX 32767
#define S16_MIN -32768
#define U32_MAX 4294967295
#define S32_MAX 2147483647
#define S32_MIN -2147483648

#define LONG_MAX S32_MAX
#define LONG_MIN S32_MIN

#define MILLION 1000000
#define BILLION 1000000000
#define TRILLION 1000000000000
#define QUADRILLION 1000000000000000

#endif
