#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

int atoi(char *str)
{
	u32 s_str = strlen(str);
	if (!s_str)
		return 0;

	u32 negative = 0;
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