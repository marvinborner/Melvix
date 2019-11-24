#include <mlibc/string.h>
#include <mlibc/stdlib.h>

char *strdup(const char *orig)
{
    size_t s_orig = strlen(orig);
    char *ret = kmalloc(s_orig + 1);
    strcpy(ret, orig);
    return ret;
}