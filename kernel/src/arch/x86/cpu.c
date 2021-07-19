// MIT License, Copyright (c) 2021 Marvin Borner

#include <cpu.h>

/**
 * Serial in/out
 */

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

/**
 * Special register manipulation
 */

CLEAR u32 cr0_get(void)
{
	u32 cr0;
	__asm__ volatile("movl %%cr0, %%eax" : "=a"(cr0));
	return cr0;
}

CLEAR void cr0_set(u32 cr0)
{
	__asm__ volatile("movl %%eax, %%cr0" ::"a"(cr0));
}

u32 cr3_get(void)
{
	u32 cr3;
	__asm__ volatile("movl %%cr0, %%eax" : "=a"(cr3));
	return cr3;
}

void cr3_set(u32 cr3)
{
	__asm__ volatile("movl %%eax, %%cr3" ::"a"(cr3));
}

CLEAR u32 cr4_get(void)
{
	u32 cr4;
	__asm__ volatile("movl %%cr4, %%eax" : "=a"(cr4));
	return cr4;
}

CLEAR void cr4_set(u32 cr4)
{
	__asm__ volatile("movl %%eax, %%cr4" ::"a"(cr4));
}
