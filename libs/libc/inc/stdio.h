// MIT License, Copyright(c) 2021 Marvin Borner

#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>
#include <stddef.h>

int vsnprintf(char *str, size_t size, const char *format, va_list ap);

#endif
