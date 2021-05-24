// MIT License, Copyright (c) 2020 Marvin Borner

#include <conv.h>
#include <def.h>
#include <math.h>
#include <mem.h>
#include <str.h>

int itoa(int value, char *buffer, int base)
{
	char tmp[16];
	char *tp = tmp;
	int i;
	unsigned v;

	int sign = (base == 10 && value < 0);
	if (sign)
		v = -value;
	else
		v = (unsigned)value;

	while (v || tp == tmp) {
		i = v % base;
		v /= base;
		if (i < 10)
			*tp++ = i + '0';
		else
			*tp++ = i + 'a' - 10;
	}

	int len = tp - tmp;

	if (sign) {
		*buffer++ = '-';
		len++;
	}

	while (tp > tmp)
		*buffer++ = *--tp;

	return len;
}

static int normalize(f64 *val)
{
	int exp = 0;
	double value = *val;

	while (value >= 1.0) {
		value /= 10.0;
		++exp;
	}

	while (value < 0.1) {
		value *= 10.0;
		--exp;
	}
	*val = value;
	return exp;
}

#define FLOAT_WIDTH 5
char *ftoa(f64 value, char *buffer)
{
	int exp = 0;
	u32 loc = 0;

	if (value == 0.0) {
		buffer[0] = '0';
		buffer[1] = '\0';
		return buffer;
	}

	if (value < 0.0) {
		*buffer++ = '-';
		value = -value;
	}

	exp = normalize(&value);

	while (exp > 0) {
		int digit = value * 10;
		*buffer++ = digit + '0';
		value = value * 10 - digit;
		++loc;
		--exp;
	}

	if (loc == 0)
		*buffer++ = '0';

	*buffer++ = '.';

	while (exp < 0 && loc < FLOAT_WIDTH) {
		*buffer++ = '0';
		--exp;
		++loc;
	}

	while (loc < FLOAT_WIDTH) {
		int digit = value * 10.0;
		*buffer++ = digit + '0';
		value = value * 10.0 - digit;
		++loc;
	}
	*buffer = '\0';

	return buffer;
}
