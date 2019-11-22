#include <mlibc/string.h>

void strcpy(char *dest, const char *orig) {
    size_t s_orig = strlen(orig);

    for (size_t i = 0; i < s_orig; i++) dest[i] = orig[i];
    dest[s_orig] = 0;
}