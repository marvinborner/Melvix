#ifndef MELVIX_STDIO_H
#define MELVIX_STDIO_H

#include <stdarg.h>

// File operations

// Character input/output
char getch();
void putch(char ch);
void puts(char *data);

void printf(char *fmt, ...);
void vprintf(char *fmt, va_list args);

#endif