// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <mem.h>
#include <print.h>

#include <core/control.h>
#include <core/features.h>
#include <core/float.h>
#include <core/io.h>

TEMPORARY static struct cpuid cpuid(u32 code)
{
	u32 a, b, c, d;
	__asm__ volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(code), "c"(0));
	return (struct cpuid){ a, b, c, d };
}

TEMPORARY static char *cpu_string(char buf[16])
{
	// wtf
	struct cpuid id = cpuid(CPUID_VENDOR_STRING);
	memcpy((u32 *)(buf + 12), &id.eax, 4);
	memcpy((u32 *)(buf + 0), &id.ebx, 4);
	memcpy((u32 *)(buf + 8), &id.ecx, 4);
	memcpy((u32 *)(buf + 4), &id.edx, 4);
	return buf;
}

TEMPORARY void cpu_print(void)
{
	char buf[16] = { 0 };
	printf("CPU vendor: %s\n", cpu_string(buf));
}

PROTECTED struct cpuid cpu_features = { 0 };
PROTECTED struct cpuid cpu_extended_information = { 0 };
PROTECTED struct cpuid cpu_extended_features = { 0 };

TEMPORARY void cpu_enable_features(void)
{
	cpu_features = cpuid(CPUID_FEATURES);
	u32 max = cpuid(0x80000000).eax;
	assert(max >= 0x80000001);
	cpu_extended_information = cpuid(0x80000001);
	cpu_extended_features = cpuid(0x7);

	// Enable NMI
	outb(0x70, inb(0x70) & 0x7F);

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
	/* if (cpu_extended_features.ecx & CPUID_EXT_FEAT_ECX_UMIP) { */
	/* 	cr4_set(cr4_get() | 0x800); */
	/* } else { */
	/* 	print("No UMIP support :(\n"); */
	/* } */

	fpu_enable();
}
