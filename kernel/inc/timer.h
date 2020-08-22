// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef TIMER_H
#define TIMER_H

#include <def.h>

u32 timer_get();
void timer_install();
void timer_handler(); // For scheduler

#endif
