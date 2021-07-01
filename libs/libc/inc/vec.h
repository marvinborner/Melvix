// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef VEC_H
#define VEC_H

#include <def.h>

/**
 * Unsigned 32
 */

typedef struct vec2 {
	u32 x, y;
} vec2;

typedef struct vec3 {
	u32 x, y, z;
} vec3;

#define vec2(x, y) ((vec2){ (x), (y) })
#define vec2to3(a, z) ((vec3){ a.x, a.y, (z) })
#define vec2_add(a, b) ((vec2){ a.x + b.x, a.y + b.y })
#define vec2_sub(a, b) ((vec2){ a.x - b.x, a.y - b.y })
#define vec2_mul(a, b) ((vec2){ a.x * (b), a.y * (b) })
#define vec2_div(a, b) ((vec2){ a.x / (b), a.y / (b) })
#define vec2_dot(a, b) ((u32)(a.x * b.x + a.y * b.y))
#define vec2_eq(a, b) (a.x == b.x && a.y == b.y)
#define vec2_sum(a) ((u32)(a.x + a.y))

#define vec3(x, y, z) ((vec3){ (x), (y), (z) })
#define vec3to2(a) ((vec2){ a.x, a.y })
#define vec3_add(a, b) ((vec3){ a.x + b.x, a.y + b.y, a.z + b.z })
#define vec3_sub(a, b) ((vec3){ a.x - b.x, a.y - b.y, a.z - b.z })
#define vec3_mul(a, b) ((vec3){ a.x * (b), a.y * (b), a.z * (b) })
#define vec3_div(a, b) ((vec3){ a.x / (b), a.y / (b), a.z / (b) })
#define vec3_dot(a, b) ((u32)(a.x * b.x + a.y * b.y + a.z * b.z))
#define vec3_eq(a, b) (a.x == b.x && a.y == b.y && a.z == c.z)
#define vec3_cross(a, b)                                                                           \
	((vec3){ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x })
#define vec3_sum(a) ((u32)(a.x + a.y + a.z))

/**
 * Floating 32
 */

typedef struct vec2f {
	f64 x, y;
} vec2f;

typedef struct vec3f {
	f64 x, y, z;
} vec3f;

#define vec2f(x, y) ((vec2f){ (x), (y) })
#define vec2fto3f(a, z) ((vec3f){ a.x, a.y, (z) })
#define vec2f_add(a, b) ((vec2f){ a.x + b.x, a.y + b.y })
#define vec2f_sub(a, b) ((vec2f){ a.x - b.x, a.y - b.y })
#define vec2f_mul(a, b) ((vec2f){ a.x * (b), a.y * (b) })
#define vec2f_div(a, b) ((vec2f){ a.x / (b), a.y / (b) })
#define vec2f_dot(a, b) ((f64)(a.x * b.x + a.y * b.y))
#define vec2f_eq(a, b) (a.x == b.x && a.y == b.y)
#define vec2f_sum(a) ((f64)(a.x + a.y))

#define vec3f(x, y, z) ((vec3f){ (x), (y), (z) })
#define vec3fto2f(a) ((vec2f){ a.x, a.y })
#define vec3f_add(a, b) ((vec3f){ a.x + b.x, a.y + b.y, a.z + b.z })
#define vec3f_sub(a, b) ((vec3f){ a.x - b.x, a.y - b.y, a.z - b.z })
#define vec3f_mul(a, b) ((vec3f){ a.x * (b), a.y * (b), a.z * (b) })
#define vec3f_div(a, b) ((vec3f){ a.x / (b), a.y / (b), a.z / (b) })
#define vec3f_dot(a, b) ((f64f)(a.x * b.x + a.y * b.y + a.z * b.z))
#define vec3f_eq(a, b) (a.x == b.x && a.y == b.y && a.z == c.z)
#define vec3f_cross(a, b)                                                                          \
	((vec3f){ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x })
#define vec3f_sum(a) ((f64)(a.x + a.y + a.z))

#endif
