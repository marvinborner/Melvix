// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef TIMER_H
#define TIMER_H

#include <def.h>

enum timer_mode {
	TIMER_MODE_DEFAULT,
	TIMER_MODE_SLEEP,
};

void timer_install_handler(void);

u32 timer_get(void);
void timer_wait(u32 ticks);
void timer_install(void);

#endif
