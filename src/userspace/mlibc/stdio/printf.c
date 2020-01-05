#include <stdarg.h>
#include <mlibc/stdio.h>
#include <mlibc/string.h>
#include <mlibc/stdlib.h>

void printf(const char *fmt, ...)
{
    char *format = (char *) malloc(strlen(fmt));
    strcpy(format, fmt);
    va_list args;
    va_start(args, fmt);
    vprintf(format, args);
    va_end(args);
}