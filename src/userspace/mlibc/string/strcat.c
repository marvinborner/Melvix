#include <mlibc/stdlib.h>

void strcat(char *dest, const char *orig)
{
    size_t s_dest = strlen(dest);
    size_t s_orig = strlen(orig);

    for (size_t i = 0; i < s_orig; i++) dest[s_dest + i] = orig[i];
    dest[s_dest + s_orig] = 0;
}