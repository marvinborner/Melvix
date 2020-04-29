#include <stdint.h>
#include <string.h>

void strcat(char *dest, char *orig)
{
	u8 s_dest = strlen(dest);
	u8 s_orig = strlen(orig);

	for (u8 i = 0; i < s_orig; i++)
		dest[s_dest + i] = orig[i];
	dest[s_dest + s_orig] = 0;
}