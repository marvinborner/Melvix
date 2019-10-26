#include <kernel/lib/lib.h>

struct idt_entry {
    unsigned short base_lo;
    unsigned short sel; // Kernel segment
    unsigned char always0; // Always 0
    unsigned char flags;
    unsigned short base_hi;
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    void *base;
} __attribute__((packed));

// Initialize IDT with 256 entries
struct idt_entry idt[256];
struct idt_ptr idtp;

// Defined in idt.asm
extern void idt_load();

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags) {
    // Specify the interrupt routine's base address
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;

    // Set selector/segment of IDT entry
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

// Install IDT
void idt_install() {
    // Set IDT pointer and limit
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = &idt;

    // Clear IDT by setting memory cells to 0
    memory_set(&idt, 0, sizeof(struct idt_entry) * 256);

    idt_load();
}
