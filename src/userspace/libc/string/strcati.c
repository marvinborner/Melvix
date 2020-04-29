#include <stdint.h>
#include <string.h>

void strcati(char *dest, char *orig)
{
	u32 s_orig = strlen(orig);
	strdisp(dest, (int)s_orig);
	for (u32 i = 0; i < s_orig; i++)
		dest[i] = orig[i];
}