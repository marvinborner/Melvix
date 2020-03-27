#ifndef MELVIX_STDIO_H
#define MELVIX_STDIO_H

#include <stdarg.h>

char getch();

char *readline();

void writec(char c);

void vprintf(const char *format, va_list args);

void printf(const char *format, ...);

#endif