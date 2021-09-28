// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef PRINT_H
#define PRINT_H

#include <arg.h>
#include <def.h>
#include <sys.h>

int printf(const char *format, ...) NONNULL;
int vprintf(const char *format, va_list ap) NONNULL;
int snprintf(char *str, u32 size, const char *format, ...) NONNULL;
int vsnprintf(char *str, u32 size, const char *format, va_list ap) NONNULL;
int print(const char *str) NONNULL;
NORETURN void panic(const char *format, ...) NONNULL;

int vfprintf(const char *path, const char *format, va_list ap) NONNULL;
int vdprintf(enum dev_type io, const char *format, va_list ap) NONNULL;
int fprintf(const char *path, const char *format, ...) NONNULL;
int dprintf(enum dev_type io, const char *format, ...) NONNULL;
int log(const char *format, ...) NONNULL;
NORETURN void err(int code, const char *format, ...) NONNULL;

#endif
