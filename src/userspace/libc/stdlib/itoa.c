#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static const char ITOA_TABLE[] = "0123456789";

char *itoa(int n)
{
	if (!n) {
		char *ret = (char *)malloc(2);
		ret[0] = '0';
		ret[1] = 0;
		return ret;
	}
	u32 negative = (u32)(n < 0);
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