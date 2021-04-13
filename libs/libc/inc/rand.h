// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef RANDOM_H
#define RANDOM_H

#include <def.h>

#define rand_range(min, max) (rand() % ((max) + 1 - (min)) + (min))

void srand(u32 seed);
u32 rand(void);
void rand_fill(void *buf, u32 size);
char *randstr(u32 size);

#endif
