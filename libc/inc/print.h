// MIT License, Copyright (c) 2020 Marvin Borner
// I may (re)move this in the future // TODO

#ifndef PRINT_H
#define PRINT_H

#include "arg.h"

int printf(const char *format, ...);
int vprintf(const char *format, va_list ap);
int vsprintf(char *str, const char *format, va_list ap);
int print(const char *str);

#ifdef userspace
int vfprintf(const char *path, const char *format, va_list ap);
int fprintf(const char *path, const char *format, ...);
int log(const char *format, ...);
int err(int code, const char *format, ...);
#else
#include <proc.h>
int print_app(enum stream_defaults id, const char *proc_name, const char *str);
#endif

#endif
