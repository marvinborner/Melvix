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
	__asm__ volatile("rep insl" ::"c"(n), // Count
			 "d"(port), // Port #
			 "D"(addr)); // Buffer
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

static void cpuid(int code, u32 *a, u32 *b, u32 *c, u32 *d)
{
	__asm__ volatile("cpuid" : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d) : "a"(code));
}

static char *cpu_string(char buf[16])
{
	// wtf
	cpuid(CPUID_VENDOR_STRING, (u32 *)(buf + 12), (u32 *)(buf), (u32 *)(buf + 8),
	      (u32 *)(buf + 4));

	return buf;
}

void cpu_print(void)
{
	char buf[16] = { 0 };
	printf("CPU vendor: %s\n", cpu_string(buf));
}

static u32 cr0_get(void)
{
	u32 cr0;
	__asm__ volatile("movl %%cr0, %%eax" : "=a"(cr0));
	return cr0;
}

static void cr0_set(u32 cr0)
{
	__asm__ volatile("movl %%eax, %%cr0" ::"a"(cr0));
}

static u32 cr4_get(void)
{
	u32 cr4;
	__asm__ volatile("movl %%cr4, %%eax" : "=a"(cr4));
	return cr4;
}

static void cr4_set(u32 cr4)
{
	__asm__ volatile("movl %%eax, %%cr4" ::"a"(cr4));
}

static u32 cpu_features = 0;
static u8 cpu_has_feature(u32 feature)
{
	return (cpu_features & feature) != 0;
}

static void fpu_handler()
{
	__asm__ volatile("clts");
}

static u8 fpu_state[512] __attribute__((aligned(16)));
void fpu_restore(void)
{
	__asm__ volatile("fxrstor (%0)" ::"r"(fpu_state));
}

void cpu_enable_features(void)
{
	u32 a, b, c, d;
	cpuid(CPUID_FEATURES, &a, &b, &c, &d);
	cpu_features = d;
	if (cpu_has_feature(CPUID_FEAT_EDX_SSE)) {
		cr0_set(cr0_get() & ~(1 << 2));
		cr0_set(cr0_get() | (1 << 1));
		cr4_set(cr4_get() | (3 << 9));
	} else {
		panic("No SSE support!\n");
	}

	if (cpu_has_feature(CPUID_FEAT_EDX_FPU)) {
		__asm__ volatile("fninit");
		__asm__ volatile("fxsave %0" : "=m"(fpu_state));
		irq_install_handler(7, fpu_handler);
	} else {
		panic("No FPU support!\n");
	}
}

void cli(void)
{
	__asm__ volatile("cli");
}

void sti(void)
{
	__asm__ volatile("sti");
}

void hlt(void)
{
	__asm__ volatile("hlt");
}

void idle(void)
{
	while (1)
		hlt();
}

void loop(void)
{
	cli();
	idle();
}
#endif
