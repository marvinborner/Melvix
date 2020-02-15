#include <stdint.h>
#include <kernel/gdt/gdt.h>
#include <kernel/system.h>
#include <kernel/lib/lib.h>
#include <kernel/memory/kheap.h>

struct gdt_entry {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed));

struct gdt_ptr {
    unsigned short limit;
    void *base;
} __attribute__((packed));

struct gdt_entry gdt[6];
struct gdt_ptr gp;

struct tss_entry_struct {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));

struct tss_entry_struct tss_entry;

extern void gdt_flush();

void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    // Set descriptor base address
    gdt[num].base_low = (unsigned short) (base & 0xFFFF);
    gdt[num].base_middle = (unsigned char) ((base >> 16) & 0xFF);
    gdt[num].base_high = (unsigned char) ((base >> 24) & 0xFF);

    // Set descriptor limits
    gdt[num].limit_low = (unsigned short) (limit & 0xFFFF);
    gdt[num].granularity = (unsigned char) ((limit >> 16) & 0x0F);

    // Set granularity and access flags
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

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
    tss_write(5, 0x10, 0x0);

    // Remove old GDT and install the new changes!
    gdt_flush();
    tss_flush();

    vga_log("Installed Global Descriptor Table");
}

void tss_write(int32_t num, uint16_t ss0, uint32_t esp0)
{
    uint32_t base = (uint32_t) &tss_entry;
    uint32_t limit = base + sizeof(tss_entry);

    gdt_set_gate(num, base, limit, 0xE9, 0x00);

    memset(&tss_entry, 0, sizeof(tss_entry));

    tss_entry.ss0 = ss0;
    tss_entry.esp0 = esp0;
    tss_entry.cs = 0x0b;
    tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x13;
}

void tss_flush()
{
    asm volatile ("ltr %%ax": : "a" (0x2B));
}

void set_kernel_stack(uintptr_t stack)
{
    tss_entry.esp0 = stack;
}