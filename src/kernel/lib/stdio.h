#ifndef MELVIX_STDIO_H
#define MELVIX_STDIO_H

#include <stdarg.h>

void putch(char c);

int sprintf(char *str, const char *fmt, ...);
void vprintf(const char *fmt, va_list args);
int vsprintf(char *str, const char *fmt, va_list args);
void printf(const char *fmt, ...);

void serial_vprintf(const char *fmt, va_list args);
void serial_printf(const char *fmt, ...);

#endif