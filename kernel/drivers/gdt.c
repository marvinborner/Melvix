// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <def.h>
#include <gdt.h>
#include <mem.h>

static struct gdt_entry gdt[6] = { 0 };
static struct tss_entry tss = { 0 };

PROTECTED static struct gdt_ptr gp = { 0 };

CLEAR static void gdt_set_gate(u32 num, u32 base, u32 limit, u8 access, u8 gran)
{
	// Set descriptor base address
	gdt[num].base_low = (u16)(base & 0xffff);
	gdt[num].base_middle = (u8)((base >> 16) & 0xff);
	gdt[num].base_high = (u8)((base >> 24) & 0xff);
	gdt[num].limit_low = (u16)(limit & 0xffff);

	// Set granularity and access flags
	gdt[num].granularity = (u8)((limit >> 16) & 0x0f);
	gdt[num].granularity |= (gran & 0xf0);
	gdt[num].access = access;
}

CLEAR static void tss_write(u32 num, u16 ss0, u32 esp0)
{
	u32 base = (u32)&tss;
	u32 limit = base + sizeof(tss);

	gdt_set_gate(num, base, limit, 0xe9, 0x00);

	memset(&tss, 0, sizeof(tss));

	tss.ss0 = ss0;
	tss.esp0 = esp0;
	tss.cs = 0x0b;
	tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13;
}

CLEAR static void tss_flush(void)
{
	__asm__ volatile("ltr %0" ::"r"((u16)0x2b));
}

CLEAR static void gdt_flush(void)
{
	__asm__ volatile("lgdt %0" ::"m"(gp) : "memory");
}

void tss_set_stack(u32 ss, u32 esp)
{
	assert(ss && esp);
	tss.esp0 = esp;
	tss.ss0 = ss;
}

CLEAR void gdt_install(void)
{
	// Set GDT pointer and limit
	gp.limit = (sizeof(struct gdt_entry) * 6) - 1;
	gp.base = &gdt;

	// NULL descriptor
	gdt_set_gate(0, 0, 0, 0, 0);

	// Code segment
	gdt_set_gate(1, 0, 0xffffffff, 0x9a, 0xcf);

	// Data segment
	gdt_set_gate(2, 0, 0xffffffff, 0x92, 0xcf);

	// User mode code segment
	gdt_set_gate(3, 0, 0xffffffff, 0xfa, 0xcf);

	// User mode data segment
	gdt_set_gate(4, 0, 0xffffffff, 0xf2, 0xcf);

	// Write TSS
	tss_write(5, GDT_SUPER_DATA_OFFSET, STACK_START);

	// Remove old GDT and install the new changes!
	gdt_flush();
	tss_flush();

	__asm__ volatile("mov %%ax, %%ds\n"
			 "mov %%ax, %%es\n"
			 "mov %%ax, %%fs\n"
			 "mov %%ax, %%gs\n"
			 "mov %%ax, %%ss\n" ::"a"(GDT_SUPER_DATA_OFFSET)
			 : "memory");

	__asm__ volatile("ljmpl $" STRINGIFY(GDT_SUPER_CODE_OFFSET) ", $code\n"
								    "code:\n");
}
