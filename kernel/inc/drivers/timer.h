// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef TIMER_H
#define TIMER_H

#include <def.h>

u32 timer_get(void);
void timer_wait(u32 ticks);
void timer_install(void);

#endif
