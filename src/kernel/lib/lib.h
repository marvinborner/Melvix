#ifndef MELVIX_LIB_H
#define MELVIX_LIB_H

#include <stddef.h>

size_t strlen(const char *str);

size_t strcmp(const char *s1, const char *s2);

char *strcat(char *dst, const char *src);

char *strcpy(char *dst, const char *src);

void *memory_copy(void *dest, const void *src, size_t count);

void *memory_set(void *dest, char val, size_t count);

int memory_compare(const void *a_ptr, const void *b_ptr, size_t size);

char *itoa(int i, char b[]);

#endif
