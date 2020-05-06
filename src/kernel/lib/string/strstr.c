#include <stdint.h>
#include <lib/stdlib.h>

char *strstr(const char *in, const char *str)
{
	char c;
	u32 len;

	c = *str++;
	if (!c)
		return (char *)in;

	len = strlen(str);
	do {
		char sc;

		do {
			sc = *in++;
			if (!sc)
				return (char *)0;
		} while (sc != c);
	} while (strncmp(in, str, len) != 0);

	return (char *)(in - 1);
}