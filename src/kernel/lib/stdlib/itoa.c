#include <lib/math.h>
#include <lib/string.h>
#include <memory/alloc.h>
#include <memory/paging.h>
#include <stdint.h>

static const char ITOA_TABLE[] = "0123456789";

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

// TODO: Rename itoa_base
char *itoa_base(int value, char *result, int base)
{
	if (base < 2 || base > 36) {
		*result = '\0';
		return result;
	}

	char *ptr = result, *ptr1 = result, tmp_char;
	int tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"
			[35 + (tmp_value - value * base)];
	} while (value);

	if (tmp_value < 0)
		*ptr++ = '-';
	*ptr-- = '\0';
	while (ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr-- = *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}