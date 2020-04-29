#ifndef MELVIX_STDIO_H
#define MELVIX_STDIO_H

#include <stdarg.h>

void putch(char c);

void vprintf(const char *fmt, va_list args);

void printf(const char *fmt, ...);

void serial_vprintf(const char *fmt, va_list args);

void serial_printf(const char *fmt, ...);

#endif