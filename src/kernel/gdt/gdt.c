#include <gdt/gdt.h>
#include <lib/lib.h>
#include <memory/alloc.h>
#include <stdint.h>
#include <system.h>

struct gdt_entry {
	u16 limit_low;
	u16 base_low;
	u8 base_middle;
	u8 access;
	u8 granularity;
	u8 base_high;
} __attribute__((packed));

struct gdt_ptr {
	u16 limit;
	void *base;
} __attribute__((packed));

struct gdt_entry gdt[6];
struct gdt_ptr gp;

struct tss_entry_struct {
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
} __attribute__((packed));

struct tss_entry_struct tss_entry;

extern void gdt_flush();

void gdt_set_gate(s32 num, u32 base, u32 limit, u8 access, u8 gran)
{
	// Set descriptor base address
	gdt[num].base_low = (u16)(base & 0xFFFF);
	gdt[num].base_middle = (u8)((base >> 16) & 0xFF);
	gdt[num].base_high = (u8)((base >> 24) & 0xFF);
	gdt[num].limit_low = (u16)(limit & 0xFFFF);

	// Set granularity and access flags
	gdt[num].granularity = (u8)((limit >> 16) & 0x0F);
	gdt[num].granularity |= (gran & 0xF0);
	gdt[num].access = access;
}

extern u32 stack_hold;

void gdt_install()
{
	// Set GDT pointer and limit
	gp.limit = (sizeof(struct gdt_entry) * 6) - 1;
	gp.base = &gdt;

	// NULL descriptor
	gdt_set_gate(0, 0, 0, 0, 0);

	// Code segment
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

	// Data segment
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

	// User mode code segment
	gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

	// User mode data segment
	gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

	// Write TSS
	tss_write(5, 0x10, stack_hold);

	gdt_set_gate(6, 0, 0xFFFFF, 0x92, 0x0);
	gdt_set_gate(7, 0, 0xFFFFF, 0x9A, 0x0);

	// Remove old GDT and install the new changes!
	gdt_flush();
	tss_flush();

	info("Installed Global Descriptor Table");
}

void tss_write(s32 num, u16 ss0, u32 esp0)
{
	u32 base = (u32)&tss_entry;
	u32 limit = base + sizeof(tss_entry);

	gdt_set_gate(num, base, limit, 0xE9, 0x00);

	memset(&tss_entry, 0, sizeof(tss_entry));

	tss_entry.ss0 = ss0;
	tss_entry.esp0 = esp0;
	tss_entry.cs = 0x0b;
	tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x13;
}

void tss_flush()
{
	asm volatile("ltr %%ax" : : "a"(0x2B));
}

void set_kernel_stack(u32 stack)
{
	tss_entry.esp0 = stack;
}