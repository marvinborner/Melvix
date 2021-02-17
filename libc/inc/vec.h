// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef VEC_H
#define VEC_H

#include <def.h>

typedef struct vec2 {
	u32 x, y;
} vec2;

typedef struct vec3 {
	u32 x, y, z;
} vec3;

#define vec2_add(a, b) ((vec2){ a.x + b.x, a.y + b.y })
#define vec2_sub(a, b) ((vec2){ a.x - b.x, a.y - b.y })
#define vec2_mul(a, b) ((vec2){ a.x * b, a.y * b })
#define vec2_div(a, b) ((vec2){ a.x / b, a.y / b })
#define vec2_dot(a, b) ((u32)(a.x * b.x + a.y * b.y))
#define vec2_eq(a, b) (a.x == b.x && a.y == b.y)

#define vec3_add(a, b) ((vec3){ a.x + b.x, a.y + b.y, a.z + b.z })
#define vec3_sub(a, b) ((vec3){ a.x - b.x, a.y - b.y, a.z - b.z })
#define vec3_mul(a, b) ((vec3){ a.x * b, a.y * b, a.z * b })
#define vec3_div(a, b) ((vec3){ a.x / b, a.y / b, a.z / b })
#define vec3_dot(a, b) ((u32)(a.x * b.x + a.y * b.y + a.z * b.z))
#define vec3_eq(a, b) (a.x == b.x && a.y == b.y && a.z == c.z)
#define vec3_cross(a, b)                                                                           \
	((vec3){ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x })

#endif
