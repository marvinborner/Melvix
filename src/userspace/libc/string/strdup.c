#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *strdup(char *orig)
{
	u8 s_orig = strlen(orig);
	char *ret = (char *)malloc(s_orig + 1);
	strcpy(ret, orig);
	return ret;
}