#include <syscall.h>
#include <mlibc/string.h>
#include <mlibc/stdio.h>

char *readline()
{
    char *ret = "";
    char buf = 0;
    while (buf != '\n') {
        buf = getch();
        writec(buf);
        strcpy(ret, buf);
    }
    strcpy(ret, buf);
    return ret;
}