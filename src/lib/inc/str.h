// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef STRING_H
#define STRING_H

#include <def.h>

u32 strlen(const char *s);
char *strcpy(char *dst, const char *src);
char *strchr(const char *s, int c);
char *strcat(char *dst, const char *src);
int strcmp(const char *s1, const char *s2);
char *strinv(char *s);
char *strdup(const char *s);

#endif
