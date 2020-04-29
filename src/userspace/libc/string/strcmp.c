#include <stdint.h>
#include <string.h>

char strcmp(char *a, char *b)
{
	if (strlen(a) != strlen(b))
		return 1;

	for (u8 i = 0; i < strlen(a); i++)
		if (a[i] != b[i])
			return 1;

	return 0;
}