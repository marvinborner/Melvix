// MIT License, Copyright (c) 2021 Marvin Borner

#include <core/descriptors/global.h>

#define GDT_MAX_LIMIT 0xffff // General max access limit without extra nibble
#define GDT_EXTRA_MAX_LIMIT (0x0f) // Extra limit nibble
#define GDT_PRESENT (1 << 7) // Makes the descriptor valid
#define GDT_DESCRIPTOR (1 << 4) // Should be set for code/data segments
#define GDT_EXECUTABLE (1 << 3) // Makes the segment executable (for code segments)
#define GDT_READWRITE (1 << 1) // Makes the segment read/writeable
#define GDT_ACCESSED (1 << 0) // Currently being accessed (for TSS)
#define GDT_SIZE (0x40) // Enable 32-Bit
#define GDT_GRANULARITY (0x80) // Enable 4K granularity for limit
#define GDT_USER (3 << 5) // Use ring 3 (user mode)

struct gdtr {
	u16 limit;
	u32 ptr;
	u32 pad;
} PACKED;

struct gdt_descriptor {
	u16 limit;
	u16 base_low;
	u8 base_mid;
	u8 access;
	u8 flags;
	u8 base_high;
} PACKED;

struct tss_descriptor {
	u32 prev_tss;
	u32 esp0;
	u32 ss0;
	u32 esp1;
	u32 ss1;
	u32 esp2;
	u32 ss2;
	u32 cr3;
	u32 eip;
	u32 eflags;
	u32 eax;
	u32 ecx;
	u32 edx;
	u32 ebx;
	u32 esp;
	u32 ebp;
	u32 esi;
	u32 edi;
	u32 es;
	u32 cs;
	u32 ss;
	u32 ds;
	u32 fs;
	u32 gs;
	u32 ldt;
	u16 trap;
	u16 iomap_base;
} PACKED;

PROTECTED static struct tss_descriptor tss_descriptor = {
	.ss0 = 0x10, // Root data
	.esp = 0, // Gets set at init
	.cs = 0x18 | 3, // Root code with ring 3
	.ss = 0x10 | 3, // Root data with ring 3
	.ds = 0x10 | 3,
	.es = 0x10 | 3,
	.fs = 0x10 | 3,
	.gs = 0x10 | 3,
	.iomap_base = sizeof(tss_descriptor),
};

PROTECTED static struct gdt_descriptor gdt_descriptors[6] = {
	// NULL descriptor // Offset 0
	{ 0 },

	// Full access code segment // Offset 1 - 0x08
	// Access from (0) to ((0xfffff << 12) + 0xfff = 4GiB)
	{
		.limit = GDT_MAX_LIMIT,
		.base_low = 0x0000,
		.base_mid = 0x00,
		.access = GDT_DESCRIPTOR | GDT_PRESENT | GDT_READWRITE | GDT_EXECUTABLE,
		.flags = GDT_GRANULARITY | GDT_SIZE | GDT_EXTRA_MAX_LIMIT,
		.base_high = 0x00,
	},

	// Full access data segment // Offset 2 - 0x10
	// Access from (0) to ((0xfffff << 12) + 0xfff = 4GiB)
	{
		.limit = GDT_MAX_LIMIT,
		.base_low = 0x0000,
		.base_mid = 0x00,
		.access = GDT_DESCRIPTOR | GDT_PRESENT | GDT_READWRITE,
		.flags = GDT_GRANULARITY | GDT_SIZE | GDT_EXTRA_MAX_LIMIT,
		.base_high = 0x00,
	},

	// User code segment // Offset 3 - 0x18
	// Access from (0) to ((0xfffff << 12) + 0xfff = 4GiB)
	{
		.limit = GDT_MAX_LIMIT,
		.base_low = 0x0000,
		.base_mid = 0x00,
		.access = GDT_DESCRIPTOR | GDT_PRESENT | GDT_USER | GDT_READWRITE | GDT_EXECUTABLE,
		.flags = GDT_GRANULARITY | GDT_SIZE | GDT_EXTRA_MAX_LIMIT,
		.base_high = 0x00,
	},

	// User data segment // Offset 4 - 0x20
	// Access from (0) to ((0xfffff << 12) + 0xfff = 4GiB)
	{
		.limit = GDT_MAX_LIMIT,
		.base_low = 0x0000,
		.base_mid = 0x00,
		.access = GDT_DESCRIPTOR | GDT_USER | GDT_USER | GDT_PRESENT | GDT_READWRITE,
		.flags = GDT_GRANULARITY | GDT_SIZE | GDT_EXTRA_MAX_LIMIT,
		.base_high = 0x00,
	},

	// Task state segment // Offset 5 - 0x28
	//
	{
		.limit = sizeof(tss_descriptor) - 1,
		.access = GDT_PRESENT | GDT_USER | GDT_EXECUTABLE | GDT_ACCESSED,
		.flags = GDT_SIZE | GDT_EXTRA_MAX_LIMIT,
	},
};

PROTECTED static struct gdtr gdtr = {
	sizeof(gdt_descriptors) - 1,
	(u32)gdt_descriptors,
	0,
};

void tss_stack_set(u32 esp)
{
	tss_descriptor.esp0 = esp;
}

TEMPORARY static void tss_init(void)
{
	struct gdt_descriptor *tss = &gdt_descriptors[5];
	tss->base_low = (u32)&tss_descriptor & 0xffff;
	tss->base_mid = ((u32)&tss_descriptor >> 16) & 0xff;
	tss->base_high = ((u32)&tss_descriptor >> 24) & 0xff;
}

TEMPORARY void gdt_init(void)
{
	tss_init();

	// Load GDT
	__asm__ volatile("lgdt %0" ::"m"(gdtr) : "memory");

	// Use second entry for all non-executable segments
	__asm__ volatile("mov %%ax, %%ds\n"
			 "mov %%ax, %%es\n"
			 "mov %%ax, %%fs\n"
			 "mov %%ax, %%gs\n"
			 "mov %%ax, %%ss\n" ::"a"(0x10) // 2nd => Data
			 : "memory");

	// Use first entry for code segment (cs register)
	__asm__ volatile("ljmpl $0x08 , $code\n"
			 "code:\n");

	// Load TSS with offset 5 (0x28) and ring 3
	__asm__ volatile("ltr %0" ::"r"((u16)(0x28 | 3)));
}
