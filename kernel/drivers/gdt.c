// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <def.h>
#include <drivers/gdt.h>
#include <mem.h>

#define GDT_MAX_LIMIT 0xffff
#define GDT_PRESENT (1 << 7)
#define GDT_RING3 (3 << 5)
#define GDT_DESCRIPTOR (1 << 4)
#define GDT_EXECUTABLE (1 << 3)
#define GDT_READWRITE (1 << 1)
#define GDT_ACCESSED (1 << 0)
#define GDT_GRANULARITY (0x80 | 0x00)
#define GDT_SIZE (0x40 | 0x00)
#define GDT_DATA_OFFSET 0x10

static struct gdt_entry gdt[6] = { 0 };
static struct tss_entry tss = { 0 };

PROTECTED static struct gdt_ptr gp = { 0 };

u8 gdt_offset(u8 gate)
{
	assert(gate && gate < COUNT(gdt));
	return ((u32)&gdt[gate] - (u32)gdt) & 0xff;
}

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
	u32 limit = sizeof(tss) - 1;

	gdt_set_gate(num, base, limit, GDT_PRESENT | GDT_RING3 | GDT_EXECUTABLE | GDT_ACCESSED,
		     GDT_SIZE);

	memset(&tss, 0, sizeof(tss));

	tss.ss0 = ss0;
	tss.esp0 = esp0;
	tss.cs = GDT_SUPER_CODE_OFFSET | 3;
	tss.ss = tss.ds = tss.es = tss.fs = tss.gs = GDT_SUPER_DATA_OFFSET | 3;
	tss.iomap_base = sizeof(tss);
}

CLEAR static void tss_flush(void)
{
	__asm__ volatile("ltr %0" ::"r"((u16)(GDT_TSS_OFFSET | 3)));
}

CLEAR static void gdt_flush(void)
{
	__asm__ volatile("lgdt %0" ::"m"(gp) : "memory");
}

void tss_set_stack(u32 esp)
{
	tss.esp0 = esp;
}

CLEAR void gdt_install(u32 esp)
{
	// Set GDT pointer and limit
	gp.limit = sizeof(gdt) - 1;
	gp.base = gdt;

	// NULL descriptor
	gdt_set_gate(0, 0, 0, 0, 0);

	// Code segment
	gdt_set_gate(GDT_ROOT_CODE_GATE, 0, U32_MAX,
		     GDT_PRESENT | GDT_DESCRIPTOR | GDT_EXECUTABLE | GDT_READWRITE,
		     GDT_GRANULARITY | GDT_SIZE);

	// Data segment
	gdt_set_gate(GDT_ROOT_DATA_GATE, 0, U32_MAX, GDT_PRESENT | GDT_DESCRIPTOR | GDT_READWRITE,
		     GDT_GRANULARITY | GDT_SIZE);

	// User mode code segment
	gdt_set_gate(GDT_USER_CODE_GATE, 0, U32_MAX,
		     GDT_PRESENT | GDT_RING3 | GDT_DESCRIPTOR | GDT_EXECUTABLE | GDT_READWRITE,
		     GDT_GRANULARITY | GDT_SIZE);

	// User mode data segment
	gdt_set_gate(GDT_USER_DATA_GATE, 0, U32_MAX,
		     GDT_PRESENT | GDT_RING3 | GDT_DESCRIPTOR | GDT_READWRITE,
		     GDT_GRANULARITY | GDT_SIZE);

	// Write TSS
	tss_write(GDT_TSS_GATE, GDT_SUPER_DATA_OFFSET, esp);

	// Remove old GDT and install the new changes!
	gdt_flush();
	__asm__ volatile("mov %%ax, %%ds\n"
			 "mov %%ax, %%es\n"
			 "mov %%ax, %%fs\n"
			 "mov %%ax, %%gs\n"
			 "mov %%ax, %%ss\n" ::"a"(GDT_SUPER_DATA_OFFSET)
			 : "memory");

	__asm__ volatile("ljmpl $0x08 , $code\n"
			 "code:\n");

	tss_flush();
}
