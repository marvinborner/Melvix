// MIT License, Copyright (c) 2020 Marvin Borner
// I may (re)move this in the future // TODO

#ifndef PRINT_H
#define PRINT_H

#include "arg.h"

int printf(const char *format, ...);
int vprintf(const char *format, va_list ap);
int vsprintf(char *str, const char *format, va_list ap);

#endif
