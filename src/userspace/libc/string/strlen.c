#include <stdint.h>

u8 strlen(char *str)
{
	u8 len = 0;
	while (str[len])
		len++;
	return len;
}