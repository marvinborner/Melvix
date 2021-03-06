// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef CPU_H
#define CPU_H

#include <def.h>
#include <proc.h>

UNUSED_FUNC static inline void spinlock(u32 *ptr)
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

void fpu_init(struct proc *proc);
void fpu_save(struct proc *proc);
void fpu_restore(struct proc *proc);

void cpu_print(void);
void cpu_enable_features(void);

u32 cr0_get(void);
void cr0_set(u32 cr0);
u32 cr3_get(void);
void cr3_set(u32 cr3);
u32 cr4_get(void);
void cr4_set(u32 cr4);

void clac(void);
void stac(void);

struct cpuid {
	u32 eax;
	u32 ebx;
	u32 ecx;
	u32 edx;
};

enum cpuid_requests {
	CPUID_VENDOR_STRING,
	CPUID_FEATURES,
	CPUID_TLB,
	CPUID_SERIAL,
	CPUID_EXT_FEATURES = 7,
};

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

	CPUID_EXT_FEAT_EBX_FSGSBASE = 1u << 0,
	CPUID_EXT_FEAT_EBX_SGX = 1u << 2,
	CPUID_EXT_FEAT_EBX_BMI1 = 1u << 3,
	CPUID_EXT_FEAT_EBX_HLE = 1u << 4,
	CPUID_EXT_FEAT_EBX_AVX2 = 1u << 5,
	CPUID_EXT_FEAT_EBX_SMEP = 1u << 7,
	CPUID_EXT_FEAT_EBX_BMI2 = 1u << 8,
	CPUID_EXT_FEAT_EBX_ERMS = 1u << 9,
	CPUID_EXT_FEAT_EBX_INVPCID = 1u << 10,
	CPUID_EXT_FEAT_EBX_RTM = 1u << 11,
	CPUID_EXT_FEAT_EBX_PQM = 1u << 12,
	CPUID_EXT_FEAT_EBX_MPX = 1u << 14,
	CPUID_EXT_FEAT_EBX_PQE = 1u << 15,
	CPUID_EXT_FEAT_EBX_AVX512F = 1u << 16,
	CPUID_EXT_FEAT_EBX_AVX512DQ = 1u << 17,
	CPUID_EXT_FEAT_EBX_RDSEED = 1u << 18,
	CPUID_EXT_FEAT_EBX_ADX = 1u << 19,
	CPUID_EXT_FEAT_EBX_SMAP = 1u << 20,
	CPUID_EXT_FEAT_EBX_AVX512IFMA = 1u << 21,
	CPUID_EXT_FEAT_EBX_PCOMMIT = 1u << 22,
	CPUID_EXT_FEAT_EBX_CLFLUSHOPT = 1u << 23,
	CPUID_EXT_FEAT_EBX_CLWB = 1u << 24,
	CPUID_EXT_FEAT_EBX_INTELPT = 1u << 25,
	CPUID_EXT_FEAT_EBX_AVX512PF = 1u << 26,
	CPUID_EXT_FEAT_EBX_AVX512ER = 1u << 27,
	CPUID_EXT_FEAT_EBX_AVX512CD = 1u << 28,
	CPUID_EXT_FEAT_EBX_SHA = 1u << 29,
	CPUID_EXT_FEAT_EBX_AVX512BW = 1u << 30,

	CPUID_EXT_FEAT_ECX_PREFETCHWT1 = 1u << 0,
	CPUID_EXT_FEAT_ECX_AVX512VBMI = 1u << 1,
	CPUID_EXT_FEAT_ECX_UMIP = 1u << 2,
	CPUID_EXT_FEAT_ECX_PKU = 1u << 3,
	CPUID_EXT_FEAT_ECX_OSPKE = 1u << 4,
	CPUID_EXT_FEAT_ECX_WAITPKG = 1u << 5,
	CPUID_EXT_FEAT_ECX_AVX512VBMI2 = 1u << 6,
	CPUID_EXT_FEAT_ECX_CETSS = 1u << 7,
	CPUID_EXT_FEAT_ECX_GFNI = 1u << 8,
	CPUID_EXT_FEAT_ECX_VAES = 1u << 9,
	CPUID_EXT_FEAT_ECX_VPCLMULQDQ = 1u << 10,
	CPUID_EXT_FEAT_ECX_AVX512VNNI = 1u << 11,
	CPUID_EXT_FEAT_ECX_AVX512BITALG = 1u << 12,
	CPUID_EXT_FEAT_ECX_AVX512VPOPCNTDQ = 1u << 14,
	CPUID_EXT_FEAT_ECX_MAWAU1 = 1u << 17,
	CPUID_EXT_FEAT_ECX_MAWAU2 = 1u << 18,
	CPUID_EXT_FEAT_ECX_MAWAU3 = 1u << 19,
	CPUID_EXT_FEAT_ECX_MAWAU4 = 1u << 20,
	CPUID_EXT_FEAT_ECX_MAWAU5 = 1u << 21,
	CPUID_EXT_FEAT_ECX_RDPID = 1u << 22,
	CPUID_EXT_FEAT_ECX_CLDEMOTE = 1u << 25,
	CPUID_EXT_FEAT_ECX_MOVDIRI = 1u << 27,
	CPUID_EXT_FEAT_ECX_MOVDIR64B = 1u << 28,
	CPUID_EXT_FEAT_ECX_ENQCMD = 1u << 29,
	CPUID_EXT_FEAT_ECX_SGXLC = 1u << 30,

	CPUID_EXT_INFO_EDX_NX = 1u << 20,
};

extern struct cpuid cpu_features;
extern struct cpuid cpu_extended_information;
extern struct cpuid cpu_extended_features;

#endif
