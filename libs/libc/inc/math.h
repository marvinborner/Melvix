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

f64 mceil(f64 x);
f64 mfloor(f64 x);

f64 mexp(f64 exp);
f64 mexp2(f64 exp);

f64 mlog(f64 x);
f64 mlog2(f64 x);

f64 mpow(f64 base, f64 exp);
f64 msqrt(f64 num);

f64 mcubic(f64 x, f64 a, f64 b, f64 c, f64 d);

f64 mlerp(f64 from, f64 to, f64 trans);
f64 mblerp(f64 a, f64 b, f64 c, f64 d, f64 transx, f64 transy);

f64 msin(f64 angle);
f64 mcos(f64 angle);
f64 mtan(f64 angle);

#endif
