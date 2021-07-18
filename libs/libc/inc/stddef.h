// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef STDDEF_H
#define STDDEF_H

#define NULL ((void *)0)
#define offsetof(type, field) ((int)(&((type *)0)->field))

typedef unsigned long size_t;
typedef long ssize_t;
typedef long ptrdiff_t;

#endif
