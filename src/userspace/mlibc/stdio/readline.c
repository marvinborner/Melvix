#include <mlibc/stdlib.h>
#include <mlibc/stdio.h>

char *readline()
{
    char ret[256] = {'\0'};
    char buf = 0;
    while (buf != '\n') {
        buf = getch();
        writec(buf);
        strcpy(ret, &buf);
    }
    return ret;
}