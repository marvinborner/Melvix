// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef CPU_H
#define CPU_H

#include <def.h>

static inline void spinlock(u32 *ptr)
{
	u32 prev;
	do
		__asm__ volatile("lock xchgl %0,%1" : "=a"(prev) : "m"(*ptr), "a"(1));
	while (prev);
}

u8 inb(u16 port);
u16 inw(u16 port);
u32 inl(u16 port);

void outb(u16 port, u8 data);
void outw(u16 port, u16 data);
void outl(u16 port, u32 data);

void cpu_print(void);
void cpu_enable_features(void);
void fpu_restore(void);

u32 cr0_get(void);
void cr0_set(u32 cr0);
u32 cr3_get(void);
void cr3_set(u32 cr3);
u32 cr4_get(void);
void cr4_set(u32 cr4);

void cli(void);
void sti(void);

enum cpuid_requests { CPUID_VENDOR_STRING, CPUID_FEATURES, CPUID_TLB, CPUID_SERIAL };
enum cpuid_features {
	CPUID_FEAT_ECX_SSE3 = 1u << 0,
	CPUID_FEAT_ECX_PCLMUL = 1u << 1,
	CPUID_FEAT_ECX_DTES64 = 1u << 2,
	CPUID_FEAT_ECX_MONITOR = 1u << 3,
	CPUID_FEAT_ECX_DS_CPL = 1u << 4,
	CPUID_FEAT_ECX_VMX = 1u << 5,
	CPUID_FEAT_ECX_SMX = 1u << 6,
	CPUID_FEAT_ECX_EST = 1u << 7,
	CPUID_FEAT_ECX_TM2 = 1u << 8,
	CPUID_FEAT_ECX_SSSE3 = 1u << 9,
	CPUID_FEAT_ECX_CID = 1u << 10,
	CPUID_FEAT_ECX_FMA = 1u << 12,
	CPUID_FEAT_ECX_CX16 = 1u << 13,
	CPUID_FEAT_ECX_ETPRD = 1u << 14,
	CPUID_FEAT_ECX_PDCM = 1u << 15,
	CPUID_FEAT_ECX_PCIDE = 1u << 17,
	CPUID_FEAT_ECX_DCA = 1u << 18,
	CPUID_FEAT_ECX_SSE4_1 = 1u << 19,
	CPUID_FEAT_ECX_SSE4_2 = 1u << 20,
	CPUID_FEAT_ECX_x2APIC = 1u << 21,
	CPUID_FEAT_ECX_MOVBE = 1u << 22,
	CPUID_FEAT_ECX_POPCNT = 1u << 23,
	CPUID_FEAT_ECX_AES = 1u << 25,
	CPUID_FEAT_ECX_XSAVE = 1u << 26,
	CPUID_FEAT_ECX_OSXSAVE = 1u << 27,
	CPUID_FEAT_ECX_AVX = 1u << 28,
	CPUID_FEAT_ECX_F16C = 1u << 29,
	CPUID_FEAT_ECX_RDRND = 1u << 30,

	CPUID_FEAT_EDX_FPU = 1u << 0,
	CPUID_FEAT_EDX_VME = 1u << 1,
	CPUID_FEAT_EDX_DE = 1u << 2,
	CPUID_FEAT_EDX_PSE = 1u << 3,
	CPUID_FEAT_EDX_TSC = 1u << 4,
	CPUID_FEAT_EDX_MSR = 1u << 5,
	CPUID_FEAT_EDX_PAE = 1u << 6,
	CPUID_FEAT_EDX_MCE = 1u << 7,
	CPUID_FEAT_EDX_CX8 = 1u << 8,
	CPUID_FEAT_EDX_APIC = 1u << 9,
	CPUID_FEAT_EDX_SEP = 1u << 11,
	CPUID_FEAT_EDX_MTRR = 1u << 12,
	CPUID_FEAT_EDX_PGE = 1u << 13,
	CPUID_FEAT_EDX_MCA = 1u << 14,
	CPUID_FEAT_EDX_CMOV = 1u << 15,
	CPUID_FEAT_EDX_PAT = 1u << 16,
	CPUID_FEAT_EDX_PSE36 = 1u << 17,
	CPUID_FEAT_EDX_PSN = 1u << 18,
	CPUID_FEAT_EDX_CLF = 1u << 19,
	CPUID_FEAT_EDX_DTES = 1u << 21,
	CPUID_FEAT_EDX_ACPI = 1u << 22,
	CPUID_FEAT_EDX_MMX = 1u << 23,
	CPUID_FEAT_EDX_FXSR = 1u << 24,
	CPUID_FEAT_EDX_SSE = 1u << 25,
	CPUID_FEAT_EDX_SSE2 = 1u << 26,
	CPUID_FEAT_EDX_SS = 1u << 27,
	CPUID_FEAT_EDX_HTT = 1u << 28,
	CPUID_FEAT_EDX_TM1 = 1u << 29,
	CPUID_FEAT_EDX_IA64 = 1u << 30,
};

u8 cpu_has_cfeature(enum cpuid_features feature);
u8 cpu_has_dfeature(enum cpuid_features feature);

#endif
