// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef STR_H
#define STR_H

#include <def.h>

u32 strlen(const char *s) NONNULL;
u32 strlcpy(char *dst, const char *src, u32 size) NONNULL;
char *strchr(char *s, int c) NONNULL;
char *strrchr(char *s, int c) NONNULL;
u32 strlcat(char *dst, const char *src, u32 size) NONNULL;
int strcmp(const char *s1, const char *s2) NONNULL;
int strncmp(const char *s1, const char *s2, u32 n) NONNULL;
char *strinv(char *s) NONNULL;
char *strdup(const char *s) NONNULL;

const char *strerror(u32 err);

#endif
