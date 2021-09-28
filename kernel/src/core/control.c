// MIT License, Copyright (c) 2020 Marvin Borner

#include <core/control.h>

TEMPORARY u32 cr0_get(void)
{
	u32 cr0;
	__asm__ volatile("movl %%cr0, %%eax" : "=a"(cr0));
	return cr0;
}

TEMPORARY void cr0_set(u32 cr0)
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

TEMPORARY u32 cr4_get(void)
{
	u32 cr4;
	__asm__ volatile("movl %%cr4, %%eax" : "=a"(cr4));
	return cr4;
}

TEMPORARY void cr4_set(u32 cr4)
{
	__asm__ volatile("movl %%eax, %%cr4" ::"a"(cr4));
}
