// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef MATH_H
#define MATH_H

#include <def.h>

/**
 * Pi constants
 */

#define M_1_PI 0.31830988618379067154
#define M_2_PI 0.63661977236758134308
#define M_2_SQRTPI 1.12837916709551257390

#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#define M_PI_4 0.78539816339744830962

f32 powf(f32 base, f32 exp);
f64 pow(f64 base, f64 exp);
f32 sqrtf(f64 num);
f64 sqrt(f64 num);

f32 sinf(f32 angle);
f64 sin(f64 angle);
f32 cosf(f32 angle);
f64 cos(f64 angle);
f32 tanf(f32 angle);
f64 tan(f64 angle);

#endif
