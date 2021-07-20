// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef STDDEF_H
#define STDDEF_H

#define NULL ((void *)0)
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define UNUSED(a) ((void)(a))
#define COUNT(a) (sizeof(a) / sizeof 0 [a])

typedef unsigned long size_t;
typedef long ssize_t;
typedef long ptrdiff_t;

#endif
