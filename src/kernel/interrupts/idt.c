#include <kernel/lib/lib.h>
#include <kernel/system.h>

struct idt_entry {
	uint16_t base_low;
	uint16_t sel; // Kernel segment
	uint8_t always0; // Always 0
	uint8_t flags;
	uint16_t base_high;
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

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags)
{
	// Specify the interrupt routine's base address
	idt[num].base_low = (uint16_t)(base & 0xFFFF);
	idt[num].base_high = (uint16_t)((base >> 16) & 0xFFFF);

	// Set selector/segment of IDT entry
	idt[num].sel = sel;
	idt[num].always0 = 0;
	idt[num].flags = (uint8_t)(flags | 0x60);
}

// Install IDT
void idt_install()
{
	// Set IDT pointer and limit
	idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
	idtp.base = &idt;

	// Clear IDT by setting memory cells to 0
	memset(&idt, 0, sizeof(struct idt_entry) * 256);

	idt_load();
	info("Installed Interrupt Descriptor Table");
}