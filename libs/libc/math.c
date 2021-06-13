// MIT License, Copyright (c) 2020 Marvin Borner

#include <math.h>

f32 powf(f32 base, f32 exp)
{
	return (f32)pow(base, exp);
}

f64 pow(f64 base, f64 exp)
{
	f64 out;
	__asm__ volatile("fyl2x;"
			 "fld %%st;"
			 "frndint;"
			 "fsub %%st,%%st(1);"
			 "fxch;"
			 "fchs;"
			 "f2xm1;"
			 "fld1;"
			 "faddp;"
			 "fxch;"
			 "fld1;"
			 "fscale;"
			 "fstp %%st(1);"
			 "fmulp;"
			 : "=t"(out)
			 : "0"(base), "u"(exp)
			 : "st(1)");
	return out;
}

// TODO: More efficient sqrt?

f32 sqrtf(f32 num)
{
	return powf(num, .5);
}

f64 sqrt(f64 num)
{
	return pow(num, .5);
}

/**
 * Interpolations
 */

f32 lerpf(f32 from, f32 to, f32 trans)
{
	return from + (to - from) * trans;
}

f64 lerp(f64 from, f64 to, f64 trans)
{
	return from + (to - from) * trans;
}

f32 blerpf(f32 a, f32 b, f32 c, f32 d, f32 transx, f32 transy)
{
	return lerpf(lerpf(a, b, transx), lerpf(c, d, transx), transy);
}

f64 blerp(f64 a, f64 b, f64 c, f64 d, f64 transx, f64 transy)
{
	return lerp(lerp(a, b, transx), lerp(c, d, transx), transy);
}

/**
 * Trigonometric functions
 */

f32 sinf(f32 angle)
{
	f32 ret = 0.0;
	__asm__ volatile("fsin" : "=t"(ret) : "0"(angle));
	return ret;
}

f64 sin(f64 angle)
{
	f64 ret = 0.0;
	__asm__ volatile("fsin" : "=t"(ret) : "0"(angle));
	return ret;
}

f32 cosf(f32 angle)
{
	return sinf(angle + (f32)M_PI_2);
}

f64 cos(f64 angle)
{
	return sin(angle + (f64)M_PI_2);
}

f32 tanf(f32 angle)
{
	return (f32)tan(angle);
}

f64 tan(f64 angle)
{
	f64 ret = 0.0, one;
	__asm__ volatile("fptan" : "=t"(one), "=u"(ret) : "0"(angle));

	return ret;
}
