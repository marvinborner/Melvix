#include <lib/string.h>

void strcat(char *dest, const char *orig)
{
	u32 s_dest = strlen(dest);
	u32 s_orig = strlen(orig);

	for (u32 i = 0; i < s_orig; i++)
		dest[s_dest + i] = orig[i];
	dest[s_dest + s_orig] = 0;
}