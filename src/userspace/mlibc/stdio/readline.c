#include <mlibc/stdlib.h>
#include <mlibc/stdio.h>

char *readline()
{
	char *ret = malloc(256);
	char buf = 0;
	while (buf != '\n') {
		buf = getch();
		writec(buf);
		strcpy(ret, &buf);
	}
	return ret;
}