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

f32 sqrtf(f64 num)
{
	return powf(num, .5);
}

f64 sqrt(f64 num)
{
	return pow(num, .5);
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
