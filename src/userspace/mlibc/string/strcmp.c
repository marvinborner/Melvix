#include <mlibc/string.h>

char strcmp(const char *a, const char *b)
{
    if (strlen(a) != strlen(b)) return 1;

    for (size_t i = 0; i < strlen(a); i++) if (a[i] != b[i]) return 1;

    return 0;
}