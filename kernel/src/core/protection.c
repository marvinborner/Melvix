// MIT License, Copyright (c) 2020 Marvin Borner

#include <core/features.h>
#include <core/protection.h>

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
