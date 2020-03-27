#ifndef MELVIX_STRING_H
#define MELVIX_STRING_H

#include <stddef.h>

size_t strlen(const char *str);

void strcpy(char *dest, const char *orig);

void strdisp(char *str, int n);

void strcat(char *dest, const char *orig);

void strcati(char *dest, const char *orig);

char strcmp(const char *a, const char *b);

char *strdup(const char *orig);

void strinv(char *str);

void *memcpy(void *dest, const void *src, size_t count);

void *memset(void *dest, char val, size_t count);

int memcmp(const void *a_ptr, const void *b_ptr, size_t size);

#endif