#include <lib/string.h>
#include <memory/alloc.h>

char *strdup(const char *orig)
{
	u32 s_orig = strlen(orig);
	char *ret = (char *)kmalloc(s_orig + 1);
	strcpy(ret, orig);
	return ret;
}