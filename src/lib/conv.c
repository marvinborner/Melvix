// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <math.h>
#include <mem.h>
#include <str.h>

static const char HTOA_TABLE[] = "0123456789ABCDEF";
static const char ITOA_TABLE[] = "0123456789";

int atoi(char *str)
{
	u32 s_str = strlen(str);
	if (!s_str)
		return 0;

	u8 negative = 0;
	if (str[0] == '-')
		negative = 1;

	u32 i = 0;
	if (negative)
		i++;

	int ret = 0;
	for (; i < s_str; i++) {
		ret += (str[i] - '0') * pow(10, (int)((s_str - i) - 1));
	}

	if (negative)
		ret *= -1;
	return ret;
}

char *htoa(u32 n)
{
	char *ret = (char *)malloc(10);

	int i = 0;
	while (n) {
		ret[i++] = HTOA_TABLE[n & 0xF];
		n >>= 4;
	}

	if (!i) {
		ret[0] = '0';
		i++;
	}

	for (; i <= 9; i++)
		ret[i] = 0;

	char *aux = strdup(ret);
	free(ret);
	ret = aux;

	strinv(ret);
	return ret;
}

int htoi(char *str)
{
	u32 s_str = strlen(str);

	u32 i = 0;
	int ret = 0;
	for (; i < s_str; i++) {
		char c = str[i];
		int aux = 0;
		if (c >= '0' && c <= '9')
			aux = c - '0';
		else if (c >= 'A' && c <= 'F')
			aux = (c - 'A') + 10;

		ret += aux * pow(16, (int)((s_str - i) - 1));
	}

	return ret;
}

char *itoa(int n)
{
	if (!n) {
		char *ret = (char *)malloc(2);
		ret[0] = '0';
		ret[1] = 0;
		return ret;
	}
	u8 negative = (u8)(n < 0);
	if (negative)
		n *= -1;

	int sz;
	for (sz = 0; n % pow(10, sz) != n; sz++) {
	}

	char *ret = (char *)malloc((u32)(sz + 1));

	for (int i = 0; i < sz; i++) {
		int digit = (n % pow(10, i + 1)) / pow(10, i);
		ret[i] = ITOA_TABLE[digit];
	}
	ret[sz] = 0;

	if (negative) {
		char *aux = (char *)malloc((u32)(sz + 2));
		strcpy(aux, ret);
		aux[sz] = '-';
		aux[sz + 1] = 0;
		free(ret);
		ret = aux;
	}

	strinv(ret);
	return ret;
}
