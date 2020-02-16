#include <kernel/lib/string.h>
#include <kernel/memory/alloc.h>

char *strdup(const char *orig)
{
    size_t s_orig = strlen(orig);
    char *ret = (char *) kmalloc(s_orig + 1);
    strcpy(ret, orig);
    return ret;
}