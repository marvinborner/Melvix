#ifndef MELVIX_STDIO_H
#define MELVIX_STDIO_H

#include <stdarg.h>

char getch();

char *readline();

void writec(char c);

void vprintf(const char *fmt, va_list args);

void printf(const char *fmt, ...);

void serial_printf(const char *fmt, ...);

#endif