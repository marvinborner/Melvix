// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef GDT_H
#define GDT_H

#include <def.h>

#define GDT_SUPER_CODE_OFFSET 0x08 // Super (kernel) code segment offset in GDT
#define GDT_SUPER_DATA_OFFSET 0x10 // Super (kernel) data segment offset in GDT
#define GDT_USER_CODE_OFFSET 0x1b // User code segment offset in GDT (with ring3 mask)
#define GDT_USER_DATA_OFFSET 0x23 // User data segment offset in GDT (with ring3 mask)

struct gdt_entry {
	u16 limit_low;
	u16 base_low;
	u8 base_middle;
	u8 access;
	u8 granularity;
	u8 base_high;
} PACKED;

struct gdt_ptr {
	u16 limit;
	void *base;
} PACKED;

struct tss_entry {
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

void gdt_install(u32 esp);
void tss_set_stack(u32 ss, u32 esp);

#endif
