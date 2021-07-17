// MIT License, Copyright (c) 2021 Marvin Borner

#include <gdt.h>
#include <kernel.h>

#define GDT_MAX_LIMIT 0xffff // General max access limit without extra nibble
#define GDT_EXTRA_MAX_LIMIT (0x0f) // Extra limit nibble
#define GDT_PRESENT (1 << 7) // Makes the descriptor valid
#define GDT_DESCRIPTOR (1 << 4) // Should be set for code/data segments
#define GDT_EXECUTABLE (1 << 3) // Makes the segment executable (for code segments)
#define GDT_READWRITE (1 << 1) // Makes the segment read/writeable
#define GDT_SIZE (0x40) // Enable 32-Bit
#define GDT_GRANULARITY (0x80) // Enable 4K granularity for limit

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

PROTECTED static struct gdt_descriptor gdt_descriptors[] = {
	// NULL descriptor // Offset 0
	{ 0 },

	// Full access code segment // Offset 1
	// Access from (0) to ((0xfffff << 12) + 0xfff = 4GiB)
	{
		.limit = GDT_MAX_LIMIT,
		.base_low = 0x0000,
		.base_mid = 0x00,
		.access = GDT_DESCRIPTOR | GDT_PRESENT | GDT_READWRITE | GDT_EXECUTABLE,
		.flags = GDT_GRANULARITY | GDT_SIZE | GDT_EXTRA_MAX_LIMIT,
		.base_high = 0x00,
	},

	// Full access data segment // Offset 2
	// Access from (0) to ((0xfffff << 12) + 0xfff = 4GiB)
	{
		.limit = GDT_MAX_LIMIT,
		.base_low = 0x0000,
		.base_mid = 0x00,
		.access = GDT_DESCRIPTOR | GDT_PRESENT | GDT_READWRITE,
		.flags = GDT_GRANULARITY | GDT_SIZE | GDT_EXTRA_MAX_LIMIT,
		.base_high = 0x00,
	},
};

PROTECTED static struct gdtr gdtr = {
	sizeof(gdt_descriptors) - 1,
	(uintptr_t)gdt_descriptors,
	0,
};

CLEAR void gdt_init(void)
{
	// Load GDT
	__asm__ volatile("lgdt %0" ::"m"(gdtr) : "memory");

	// Use second entry for all non-executable segments
	__asm__ volatile("mov %%ax, %%ds\n"
			 "mov %%ax, %%es\n"
			 "mov %%ax, %%fs\n"
			 "mov %%ax, %%gs\n"
			 "mov %%ax, %%ss\n" ::"a"(GDT_DATA_SEGMENT)
			 : "memory");

	// Use first entry for code segment (cs register)
	__asm__ volatile("ljmpl $0x08 , $code\n"
			 "code:\n");
}
