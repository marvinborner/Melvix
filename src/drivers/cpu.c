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
