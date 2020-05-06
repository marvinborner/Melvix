#include <lib/lib.h>
#include <system.h>

struct idt_entry {
	u16 base_low;
	u16 sel; // Kernel segment
	u8 always0; // Always 0
	u8 flags;
	u16 base_high;
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
	idt[num].base_low = (u16)(base & 0xFFFF);
	idt[num].base_high = (u16)((base >> 16) & 0xFFFF);

	// Set selector/segment of IDT entry
	idt[num].sel = sel;
	idt[num].always0 = 0;
	idt[num].flags = (u8)(flags | 0x60);
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