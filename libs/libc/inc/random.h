// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef RANDOM_H
#define RANDOM_H

#include <def.h>

void srand(u32 seed);
u32 rdrand(void);
u32 rdseed(void);
u32 rand(void);
char *randstr(u32 size);

#endif
