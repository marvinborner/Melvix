// MIT License, Copyright(c) 2021 Marvin Borner

#ifndef STRING_H
#define STRING_H

#include <stddef.h>

size_t strlen(const char *s);
size_t strnlen(const char *s, size_t maxlen);
size_t strlcpy(char *dst, const char *src, size_t size);
size_t strlcat(char *dst, const char *src, size_t size);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *dst, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

#endif
