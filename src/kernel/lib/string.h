#ifndef MELVIX_STRING_H
#define MELVIX_STRING_H

#include <stddef.h>
#include <stdint.h>

u32 strlen(const char *str);

void strcpy(char *dest, const char *orig);

void strdisp(char *str, int n);

void strcat(char *dest, const char *orig);

void strcati(char *dest, const char *orig);

char strcmp(const char *a, const char *b);

int strncmp(const char *s1, const char *s2, int c);

char *strdup(const char *orig);

void strinv(char *str);

char *strstr(const char *in, const char *str);

char *strsep(char **stringp, const char *delim);

#endif