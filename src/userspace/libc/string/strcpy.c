#include <stdint.h>
#include <string.h>

void strcpy(char *dest, char *orig)
{
	u8 s_orig = strlen(orig);

	for (u8 i = 0; i < s_orig; i++)
		dest[i] = orig[i];
	dest[s_orig] = 0;
}