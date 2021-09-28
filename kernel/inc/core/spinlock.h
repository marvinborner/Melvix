// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef CORE_SPINLOCK_H
#define CORE_SPINLOCK_H

#include <def.h>

UNUSED_FUNC static inline void spinlock(u32 *ptr)
{
	u32 prev;
	do
		__asm__ volatile("lock xchgl %0,%1" : "=a"(prev) : "m"(*ptr), "a"(1));
	while (prev);
}

#endif
