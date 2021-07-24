// MIT License, Copyright(c) 2021 Marvin Borner

#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>
#include <stddef.h>
#include <sys/dev.h>

int vdprintf(dev_t type, const char *format, va_list ap);
int dprintf(dev_t type, const char *format, ...);

int vfprintf(const char *path, const char *format, va_list ap);
int fprintf(const char *path, const char *format, ...);

int vsnprintf(char *str, size_t size, const char *format, va_list ap);
int snprintf(char *str, size_t size, const char *format, ...);
int vprintf(const char *format, va_list ap);
int printf(const char *format, ...);

#endif
