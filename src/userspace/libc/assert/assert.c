#include <stdio.h>
#include <stdlib.h>

void _assert(const char *file, int line, const char *func, const char *exp)
{
	printf("%s:%d: %s: Assertion '%s' failed", file, line, func, exp);
	exit(1);
}