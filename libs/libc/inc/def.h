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

#define ABS(a) ((u32)(((s32)(a) < 0) ? (-a) : (a)))

#define NORETURN __attribute__((noreturn))
#define DEPRECATED __attribute__((deprecated))
#define NO_SANITIZE __attribute__((no_sanitize("undefined")))
#define PACKED __attribute__((packed))
#define ALIGNED(align) __attribute__((aligned(align)))

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
