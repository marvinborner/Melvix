// MIT License, Copyright (c) 2020 Marvin Borner

#include <math.h>

f64 mceil(f64 x)
{
	if (x == 0.0)
		return x;

	f64 out;
	__asm__ volatile("frndint\n" : "=t"(out) : "0"(x));
	if (out < x)
		return out + 1.0;
	return out;
}

f64 mfloor(f64 x)
{
	if (x == 0.0)
		return x;

	f64 out;
	__asm__ volatile("frndint\n" : "=t"(out) : "0"(x));
	if (out > x)
		return out - 1.0;
	return out;
}

f64 mexp(f64 exp)
{
	f64 out;
	__asm__ volatile("fldl2e\n"
			 "fmulp\n"
			 "fld1\n"
			 "fld %%st(1)\n"
			 "fprem\n"
			 "f2xm1\n"
			 "faddp\n"
			 "fscale\n"
			 "fstp %%st(1)"
			 : "=t"(out)
			 : "0"(exp));
	return out;
}

f64 mexp2(f64 exp)
{
	f64 out;
	__asm__ volatile("fld1\n"
			 "fld %%st(1)\n"
			 "fprem\n"
			 "f2xm1\n"
			 "faddp\n"
			 "fscale\n"
			 "fstp %%st(1)"
			 : "=t"(out)
			 : "0"(exp));
	return out;
}

f64 mlog(f64 x)
{
	f64 out;
	__asm__ volatile("fldln2\n"
			 "fld %%st(1)\n"
			 "fyl2x\n"
			 "fstp %%st(1)"
			 : "=t"(out)
			 : "0"(x));
	return out;
}

f64 mlog2(f64 x)
{
	f64 out;
	__asm__ volatile("fld1\n"
			 "fld %%st(1)\n"
			 "fyl2x\n"
			 "fstp %%st(1)"
			 : "=t"(out)
			 : "0"(x));
	return out;
}

f64 mpow(f64 base, f64 exp)
{
	if (exp == 0)
		return 1;
	if (exp == 1)
		return base;
	if (base == 0)
		return 0;
	if (exp == (f64)((s32)exp)) {
		f64 out = base;
		for (u32 i = 0; i < FABS(exp) - 1; i++)
			out *= base;
		if (exp < 0)
			out = 1.0 / out;
		return out;
	}
	return mexp2(exp * mlog2(base));
}

// TODO: More efficient sqrt?

f64 msqrt(f64 num)
{
	return mpow(num, .5);
}

f64 mcubic(f64 x, f64 a, f64 b, f64 c, f64 d)
{
	return a * mpow(x, 3) + b * mpow(x, 2) + c * x + d;
}

/**
 * Interpolations
 */

f64 mlerp(f64 from, f64 to, f64 trans)
{
	return from + (to - from) * trans;
}

f64 mblerp(f64 a, f64 b, f64 c, f64 d, f64 transx, f64 transy)
{
	return mlerp(mlerp(a, b, transx), mlerp(c, d, transx), transy);
}

/**
 * Trigonometric functions
 */

f64 msin(f64 angle)
{
	f64 ret = 0.0;
	__asm__ volatile("fsin" : "=t"(ret) : "0"(angle));
	return ret;
}

f64 mcos(f64 angle)
{
	return msin(angle + (f64)M_PI_2);
}

f64 mtan(f64 angle)
{
	f64 ret = 0.0, one;
	__asm__ volatile("fptan" : "=t"(one), "=u"(ret) : "0"(angle));

	return ret;
}
