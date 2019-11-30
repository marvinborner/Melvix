#ifndef MELVIX_STDIO_H
#define MELVIX_STDIO_H

// TODO: Input methods

#include <stdarg.h>

void writec(char c);

void vprintf(const char *format, va_list args);

void printf(const char *format, ...);

#endif
