// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef CPU_H
#define CPU_H

#include <def.h>

enum cpuid_requests { CPUID_VENDOR_STRING, CPUID_FEATURES, CPUID_TLB, CPUID_SERIAL };

u8 inb(u16 port);
u16 inw(u16 port);
u32 inl(u16 port);
void insl(u16 port, void *addr, int n);

void outb(u16 port, u8 data);
void outw(u16 port, u16 data);
void outl(u16 port, u32 data);

void cpuid(int code, u32 *a, u32 *b, u32 *c, u32 *d);
char *cpu_string(char buf[12]);
void cpu_print(void);

#ifdef kernel
void cli(void);
void sti(void);
void hlt(void);
void idle(void);
void loop(void);
#endif

static inline void spinlock(int *ptr)
{
	int prev;
	do
		__asm__ volatile("lock xchgl %0,%1" : "=a"(prev) : "m"(*ptr), "a"(1));
	while (prev);
}

#endif
