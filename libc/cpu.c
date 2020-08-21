// MIT License, Copyright (c) 2020 Marvin Borner
// This file is a wrapper around some CPU asm calls

#include <def.h>

u8 inb(u16 port)
{
	u8 value;
	__asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
	return value;
}

u16 inw(u16 port)
{
	u16 value;
	__asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
	return value;
}

u32 inl(u16 port)
{
	u32 value;
	__asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
	return value;
}

void insl(u16 port, void *addr, int n)
{
	__asm__ volatile("cld; rep insl"
			 : "=D"(addr), "=c"(n)
			 : "d"(port), "0"(addr), "1"(n)
			 : "memory", "cc");
}

void outb(u16 port, u8 data)
{
	__asm__ volatile("outb %0, %1" ::"a"(data), "Nd"(port));
}

void outw(u16 port, u16 data)
{
	__asm__ volatile("outw %0, %1" ::"a"(data), "Nd"(port));
}

void outl(u16 port, u32 data)
{
	__asm__ volatile("outl %0, %1" ::"a"(data), "Nd"(port));
}

#ifdef kernel
void cli()
{
	__asm__ volatile("cli");
}

void sti()
{
	__asm__ volatile("sti");
}

void hlt()
{
	__asm__ volatile("hlt");
}

void idle()
{
	while (1)
		hlt();
}

void loop()
{
	cli();
	idle();
}
#endif
