#include <stdint.h>

u32 strlen(char *str)
{
	u32 len = 0;
	while (str[len])
		len++;
	return len;
}