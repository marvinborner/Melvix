// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef CONV_H
#define CONV_H

#include <def.h>

int atoi(const char *str);
char *htoa(u32 n);
int htoi(const char *str);
char *itoa(int n);

char *conv_base(int value, char *result, int base, int is_signed);

#endif
