#include <lib/string.h>

void strcpy(char *dest, const char *orig)
{
	u32 s_orig = strlen(orig);

	for (u32 i = 0; i < s_orig; i++)
		dest[i] = orig[i];
	dest[s_orig] = 0;
}