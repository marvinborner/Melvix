#ifndef MELVIX_STRING_H
#define MELVIX_STRING_H

#include <stddef.h>
#include <stdint.h>

u8 strlen(char *str);

void strcpy(char *dest, char *orig);

void strdisp(char *str, int n);

void strcat(char *dest, char *orig);

void strcati(char *dest, char *orig);

char strcmp(char *a, char *b);

int strncmp(char *s1, char *s2, int c);

char *strdup(char *orig);

void strinv(char *str);

char *strstr(char *in, char *str);

char *strsep(char **stringp, char *delim);

#endif