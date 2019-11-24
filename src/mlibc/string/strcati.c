#include <mlibc/string.h>

void strcati(char *dest, const char *orig)
{
    size_t s_orig = strlen(orig);
    strdisp(dest, (int) s_orig);
    for (size_t i = 0; i < s_orig; i++) dest[i] = orig[i];
}