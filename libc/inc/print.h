// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef PRINT_H
#define PRINT_H

#include "arg.h"
#include <def.h>

int printf(const char *format, ...);
int vprintf(const char *format, va_list ap);
int sprintf(char *str, const char *format, ...);
int vsprintf(char *str, const char *format, va_list ap);
int print(const char *str);
NORETURN void panic(const char *format, ...);

#ifdef userspace
int vfprintf(const char *path, const char *format, va_list ap);
int fprintf(const char *path, const char *format, ...);
int log(const char *format, ...);
int err(int code, const char *format, ...);
#else
#include <proc.h>
int print_app(enum stream_defaults id, const char *proc_name, const char *str);
void print_trace(u32 count);
#endif

#endif
