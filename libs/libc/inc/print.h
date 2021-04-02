// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef PRINT_H
#define PRINT_H

#include <arg.h>
#include <def.h>

int printf(const char *format, ...) NONNULL;
int vprintf(const char *format, va_list ap) NONNULL;
int snprintf(char *str, u32 size, const char *format, ...) NONNULL;
int vsnprintf(char *str, u32 size, const char *format, va_list ap) NONNULL;
int print(const char *str) NONNULL;
NORETURN void panic(const char *format, ...) NONNULL;

#ifdef USER
int vfprintf(const char *path, const char *format, va_list ap) NONNULL;
int fprintf(const char *path, const char *format, ...) NONNULL;
int log(const char *format, ...) NONNULL;
void err(int code, const char *format, ...) NONNULL;
#else
#include <proc.h>
int print_app(enum stream_defaults id, const char *proc_name, const char *str) NONNULL;
void print_trace(u32 count);
#endif

#endif
