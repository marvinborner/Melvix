// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>

void set_fpu_cw(const u16 cw)
{
	__asm__ volatile("fldcw %0" ::"m"(cw));
}

void fpu_install(void)
{
	__asm__ volatile("clts");
	u32 t = 0;
	__asm__ volatile("mov %%cr0, %0" : "=r"(t));
	t &= (u32) ~(1 << 2);
	t |= (1 << 1);
	__asm__ volatile("mov %0, %%cr0" ::"r"(t));

	__asm__ volatile("mov %%cr4, %0" : "=r"(t));
	t |= 3 << 9;
	__asm__ volatile("mov %0, %%cr4" ::"r"(t));

	__asm__ volatile("fninit");
}
