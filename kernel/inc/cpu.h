// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef CPU_H
#define CPU_H

#include <def.h>

u8 inb(u16 port);
u16 inw(u16 port);
u32 inl(u16 port);
void insl(u16 port, void *addr, int n);

void outb(u16 port, u8 data);
void outw(u16 port, u16 data);
void outl(u16 port, u32 data);
void cli();
void sti();
void hlt();
void idle();

static inline void spinlock(int *ptr)
{
	int prev;
	do
		__asm__ volatile("lock xchgl %0,%1" : "=a"(prev) : "m"(*ptr), "a"(1));
	while (prev);
}

#endif
