// MIT License, Copyright (c) 2020 Marvin Borner

#include <cpu.h>
#include <def.h>
#include <interrupts.h>
#include <serial.h>

/**
 * IDT
 */

void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags)
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
	idt_ptr.limit = (sizeof(struct idt_entry) * 256) - 1;
	idt_ptr.base = &idt;

	// Clear IDT by setting memory cells to 0
	//memset(&idt, 0, sizeof(struct idt_entry) * 256);

	__asm__("lidt %0" : : "m"(idt_ptr));
}

/**
 * IRQ
 */

void (*irq_routines[16])(struct regs *) = { 0 };

// Install IRQ handler
void irq_install_handler(int irq, void (*handler)(struct regs *r))
{
	irq_routines[irq] = handler;
}

// Remove IRQ handler
void irq_uninstall_handler(int irq)
{
	irq_routines[irq] = 0;
}

// Remap the IRQ table
void irq_remap()
{
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
	outb(0x21, 0x00);
	outb(0xA1, 0x00);
}

// Handle IRQ ISRs
__attribute__((interrupt)) void irq_handler(struct regs *r)
{
	void (*handler)(struct regs * r);

	// Execute custom handler if exists
	handler = irq_routines[r->int_no - 32];
	if (handler)
		handler(r);

	// Send EOI to second (slave) PIC
	if (r->int_no >= 40)
		outb(0xA0, 0x20);

	// Send EOI to master PIC
	outb(0x20, 0x20);
}

// Map ISRs to the correct entries in the IDT
void irq_install()
{
	irq_remap();
	for (int i = 32; i < 48; i++)
		idt_set_gate(i, (u32)irq_handler, 0x08, 0x8E);
}

/**
 * ISR
 */

void (*isr_routines[256])(struct regs *) = { 0 };

void isr_install_handler(int isr, void (*handler)(struct regs *r))
{
	isr_routines[isr] = handler;
}

void isr_uninstall_handler(int isr)
{
	isr_routines[isr] = 0;
}

__attribute__((interrupt)) void isr_handler(struct regs *r)
{
	void (*handler)(struct regs * r);

	// Execute fault handler if exists
	handler = isr_routines[r->int_no];
	if (handler) {
		handler(r);
	} else {
		serial_print("Got ISR!\n");
		__asm__("cli");
		while (1) {
		};
	}
}

void isr_install()
{
	for (int i = 0; i < 32; i++)
		idt_set_gate(i, (u32)isr_handler, 0x08, 0x8E);
}

/**
 * Combined
 */
void interrupts_install()
{
	idt_install();
	isr_install();
	irq_install();
	__asm__("sti");
}
