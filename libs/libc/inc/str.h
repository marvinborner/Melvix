// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef STR_H
#define STR_H

#include <def.h>

u32 strlen(const char *s);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, u32 n);
char *strchr(char *s, int c);
char *strrchr(char *s, int c);
char *strcat(char *dst, const char *src);
char *strncat(char *dst, const char *src, u32 n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, u32 n);
char *strinv(char *s);
char *strdup(const char *s);

const char *strerror(u32 err);

#endif
