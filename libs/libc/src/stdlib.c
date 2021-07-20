// MIT License, Copyright(c) 2021 Marvin Borner

#include <stdlib.h>

int itoa(int value, char *buffer, unsigned int base)
{
	char tmp[16];
	char *tp = tmp;
	unsigned v;

	int sign = (base == 10 && value < 0);
	if (sign)
		v = -value;
	else
		v = (unsigned)value;

	while (v || tp == tmp) {
		int i = v % base;
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
	*buffer = '\0';

	return len;
}
