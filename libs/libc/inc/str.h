// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef STR_H
#define STR_H

#include <def.h>

PURE u32 strlen(const char *s) NONNULL;
PURE u32 strnlen(const char *s, u32 max) NONNULL;
u32 strlcpy(char *dst, const char *src, u32 size) NONNULL;
PURE const char *strcchr(const char *s, char c) NONNULL;
PURE const char *strrcchr(const char *s, char c) NONNULL;
PURE char *strchr(char *s, char c) NONNULL;
PURE char *strrchr(char *s, char c) NONNULL;
u32 strlcat(char *dst, const char *src, u32 size) NONNULL;
s32 strcmp(const char *s1, const char *s2) NONNULL;
s32 strncmp(const char *s1, const char *s2, u32 n) NONNULL;
char *strinv(char *s) NONNULL;
ATTR((malloc)) char *strdup(const char *s) NONNULL;
const char *strerror(u32 err);

#endif
