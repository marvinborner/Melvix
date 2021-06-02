// MIT License, Copyright (c) 2020 Marvin Borner
// This file is a wrapper around some CPU asm calls

#include <assert.h>
#include <def.h>
#include <drivers/cpu.h>
#include <mem.h>
#include <print.h>

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

/**
 * FPU
 */

PROTECTED static u8 fpu_initial[512] ALIGNED(16);
static u8 fpu_regs[512] ALIGNED(16);

static void fpu_handler(void)
{
	__asm__ volatile("clts");
}

void fpu_init(struct proc *proc)
{
	memcpy(&proc->fpu, &fpu_initial, sizeof(fpu_initial));
}

void fpu_save(struct proc *proc)
{
	__asm__ volatile("fxsave (%0)" ::"r"(fpu_regs));
	memcpy(&proc->fpu, &fpu_regs, sizeof(fpu_regs));
}

void fpu_restore(struct proc *proc)
{
	memcpy(&fpu_regs, &proc->fpu, sizeof(proc->fpu));
	__asm__ volatile("fxrstor (%0)" ::"r"(fpu_regs));
}

/**
 * CPU features
 */

CLEAR static struct cpuid cpuid(u32 code)
{
	u32 a, b, c, d;
	__asm__ volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(code), "c"(0));
	return (struct cpuid){ a, b, c, d };
}

CLEAR static char *cpu_string(char buf[16])
{
	// wtf
	struct cpuid id = cpuid(CPUID_VENDOR_STRING);
	memcpy((u32 *)(buf + 12), &id.eax, 4);
	memcpy((u32 *)(buf + 0), &id.ebx, 4);
	memcpy((u32 *)(buf + 8), &id.ecx, 4);
	memcpy((u32 *)(buf + 4), &id.edx, 4);
	return buf;
}

CLEAR void cpu_print(void)
{
	char buf[16] = { 0 };
	printf("CPU vendor: %s\n", cpu_string(buf));
}

PROTECTED struct cpuid cpu_features = { 0 };
PROTECTED struct cpuid cpu_extended_information = { 0 };
PROTECTED struct cpuid cpu_extended_features = { 0 };

CLEAR void cpu_enable_features(void)
{
	cpu_features = cpuid(CPUID_FEATURES);
	u32 max = cpuid(0x80000000).eax;
	assert(max >= 0x80000001);
	cpu_extended_information = cpuid(0x80000001);
	cpu_extended_features = cpuid(0x7);

	// Enable SSE
	if (cpu_features.edx & CPUID_FEAT_EDX_SSE) {
		__asm__ volatile("clts");
		cr0_set(cr0_get() & ~(1 << 2));
		cr0_set(cr0_get() | (1 << 1));
		cr0_set(cr0_get() | (1 << 5));
		cr4_set(cr4_get() | (3 << 9));
	} else {
		panic("No SSE support!\n");
	}

	// Enable FPU
	if (cpu_features.edx & CPUID_FEAT_EDX_FPU) {
		__asm__ volatile("fninit");
		__asm__ volatile("fxsave %0" : "=m"(fpu_initial));
		int_event_handler_add(7, fpu_handler);
	} else {
		panic("No FPU support!\n");
	}

	// Enable NX (IA32_EFER.NXE) // TODO: Use NX Bit? (only possible in PAE 64 bit paging?)
	if (cpu_extended_information.edx & CPUID_EXT_INFO_EDX_NX) {
		__asm__ volatile("movl $0xc0000080, %ecx\n"
				 "rdmsr\n"
				 "orl $0x800, %eax\n"
				 "wrmsr\n");
	} else {
		print("No NX support :(\n");
	}

	// Enable SMEP
	if (cpu_extended_features.ebx & CPUID_EXT_FEAT_EBX_SMEP) {
		cr4_set(cr4_get() | 0x100000);
	} else {
		print("No SMEP support :(\n");
	}

	// Enable SMAP
	if (cpu_extended_features.ebx & CPUID_EXT_FEAT_EBX_SMAP) {
		cr4_set(cr4_get() | 0x200000);
	} else {
		print("No SMAP support :(\n");
	}

	// Enable UMIP // TODO: QEMU support?!
	if (cpu_extended_features.ecx & CPUID_EXT_FEAT_ECX_UMIP) {
		cr4_set(cr4_get() | 0x800);
	} else {
		print("No UMIP support :(\n");
	}
}

/**
 * SMAP
 */

void clac(void)
{
	if (cpu_extended_features.ebx & CPUID_EXT_FEAT_EBX_SMAP)
		__asm__ volatile("clac" ::: "cc");
}

void stac(void)
{
	if (cpu_extended_features.ebx & CPUID_EXT_FEAT_EBX_SMAP)
		__asm__ volatile("stac" ::: "cc");
}
