// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef STR_H
#define STR_H

#include <def.h>

u32 strlen(const char *s) NONNULL;
u32 strlcpy(char *dst, const char *src, u32 size) NONNULL;
char *strchr(char *s, char c) NONNULL;
char *strrchr(char *s, char c) NONNULL;
u32 strlcat(char *dst, const char *src, u32 size) NONNULL;
s32 strcmp(const char *s1, const char *s2) NONNULL;
s32 strncmp(const char *s1, const char *s2, u32 n) NONNULL;
char *strinv(char *s) NONNULL;
char *strdup(const char *s) NONNULL;

#ifdef KERNEL

u32 strlen_user(const char *s) NONNULL;
u32 strlcpy_user(char *dst, const char *src, u32 size) NONNULL;
char *strchr_user(char *s, char c) NONNULL;
char *strrchr_user(char *s, char c) NONNULL;
u32 strlcat_user(char *dst, const char *src, u32 size) NONNULL;
s32 strcmp_user(const char *s1, const char *s2) NONNULL;
s32 strncmp_user(const char *s1, const char *s2, u32 n) NONNULL;
char *strinv_user(char *s) NONNULL;
char *strdup_user(const char *s) NONNULL;

#endif

const char *strerror(u32 err);

#endif
