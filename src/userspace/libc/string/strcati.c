#include <stdint.h>
#include <string.h>

void strcati(char *dest, char *orig)
{
	u8 s_orig = strlen(orig);
	strdisp(dest, (int)s_orig);
	for (u8 i = 0; i < s_orig; i++)
		dest[i] = orig[i];
}