// MIT License, Copyright (c) 2020 Marvin Borner
// This file is a wrapper around some CPU asm calls

#include <cpu.h>
#include <def.h>
#include <print.h>

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

void cpuid(int code, u32 *a, u32 *b, u32 *c, u32 *d)
{
	__asm__ volatile("cpuid" : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d) : "a"(code));
}

char *cpu_string(char buf[13])
{
	u32 a, b, c, d;
	cpuid(CPUID_VENDOR_STRING, &a, &b, &c, &d);
	char *ebx = (char *)&b;
	char *ecx = (char *)&c;
	char *edx = (char *)&d;
	buf[0] = ebx[0];
	buf[1] = ebx[1];
	buf[2] = ebx[2];
	buf[3] = ebx[3];
	buf[4] = edx[0];
	buf[5] = edx[1];
	buf[6] = edx[2];
	buf[7] = edx[3];
	buf[8] = ecx[0];
	buf[9] = ecx[1];
	buf[10] = ecx[2];
	buf[11] = ecx[3];
	buf[12] = 0;
	return buf;
}

void cpu_print()
{
	char buf[13] = { 0 };
	printf("%s\n", cpu_string(buf));
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
