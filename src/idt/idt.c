#include "../memory/memory.h"

/* Defines an IDT entry */
struct idt_entry {
    unsigned short base_lo;
    unsigned short sel; // Kernel segment
    unsigned char always0; // Always 0
    unsigned char flags;
    unsigned short base_hi;
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

// Initialize IDT with 256 entries
struct idt_entry idt[256];
struct idt_ptr idtp;

// Defined in boot.asm
extern void idt_load();

// Install IDT
void idt_install() {
    // Set IDT pointer and limit
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = &idt;

    // Clear IDT by setting memory cells to 0
    memory_set(&idt, 0, sizeof(struct idt_entry) * 256);

    // TODO: Add method to add ISRs to IDT

    idt_load();
}
