#include <mlibc/math.h>
#include <stddef.h>
#include <stdint.h>
#include <mlibc/stdlib.h>

int atoi(char *str)
{
	size_t s_str = strlen(str);
	if (!s_str)
		return 0;

	uint8_t negative = 0;
	if (str[0] == '-')
		negative = 1;

	size_t i = 0;
	if (negative)
		i++;

	int ret = 0;
	for (; i < s_str; i++) {
		ret += (str[i] - '0') * pow(10, (s_str - i) - 1);
	}

	if (negative)
		ret *= -1;
	return ret;
}