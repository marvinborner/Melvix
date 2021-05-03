// MIT License, Copyright (c) 2020 Marvin Borner
// TODO: Remove some magic numbers

#include <assert.h>
#include <cpu.h>
#include <def.h>
#include <interrupts.h>
#include <mem.h>
#include <mm.h>
#include <print.h>
#include <proc.h>
#include <serial.h>

/**
 * IDT
 */

PROTECTED static struct idt_entry idt[256] = { 0 };
PROTECTED static struct idt_ptr idt_ptr = { 0 };

CLEAR void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags)
{
	// Specify the interrupt routine's base address
	idt[num].base_low = (u16)(base & 0xffff);
	idt[num].base_high = (u16)((base >> 16) & 0xffff);

	// Set selector/segment of IDT entry
	idt[num].sel = sel;
	idt[num].always0 = 0;
	idt[num].flags = flags;
}

// Install IDT
CLEAR static void idt_install(void)
{
	// Set IDT pointer and limit
	idt_ptr.limit = sizeof(idt) - 1;
	idt_ptr.base = &idt;

	// Clear IDT by setting memory cells to 0
	memset(&idt, 0, sizeof(idt));

	__asm__ volatile("lidt %0" : : "m"(idt_ptr));
}

/**
 * IRQ
 */

PROTECTED static void (*irq_routines[16])(struct regs *) = { 0 };

// Install IRQ handler
CLEAR void irq_install_handler(int irq, void (*handler)(struct regs *r))
{
	irq_routines[irq] = handler;
}

// Remove IRQ handler
CLEAR void irq_uninstall_handler(int irq)
{
	irq_routines[irq] = 0;
}

// Remap the IRQ table
CLEAR static void irq_remap(void)
{
	outb(0x20, 0x11);
	outb(0xa0, 0x11);
	outb(0x21, 0x20);
	outb(0xa1, 0x28);
	outb(0x21, 0x04);
	outb(0xa1, 0x02);
	outb(0x21, 0x01);
	outb(0xa1, 0x01);
	outb(0x21, 0x00);
	outb(0xa1, 0x00);
}

// Handle IRQ ISRs
void irq_handler(struct regs *r);
void irq_handler(struct regs *r)
{
	void (*handler)(struct regs * r);

	// Execute custom handler if exists
	handler = irq_routines[r->int_no - 32];
	if (handler)
		handler(r);

	// Send EOI to second (slave) PIC
	if (r->int_no >= 40)
		outb(0xa0, 0x20);

	// Send EOI to master PIC
	outb(0x20, 0x20);
}

// Map ISRs to the correct entries in the IDT
CLEAR static void irq_install(void)
{
	irq_remap();

	idt_set_gate(32, (u32)irq0, 0x08, 0x8e);
	idt_set_gate(33, (u32)irq1, 0x08, 0x8e);
	idt_set_gate(34, (u32)irq2, 0x08, 0x8e);
	idt_set_gate(35, (u32)irq3, 0x08, 0x8e);
	idt_set_gate(36, (u32)irq4, 0x08, 0x8e);
	idt_set_gate(37, (u32)irq5, 0x08, 0x8e);
	idt_set_gate(38, (u32)irq6, 0x08, 0x8e);
	idt_set_gate(39, (u32)irq7, 0x08, 0x8e);

	idt_set_gate(40, (u32)irq8, 0x08, 0x8e);
	idt_set_gate(41, (u32)irq9, 0x08, 0x8e);
	idt_set_gate(42, (u32)irq10, 0x08, 0x8e);
	idt_set_gate(43, (u32)irq11, 0x08, 0x8e);
	idt_set_gate(44, (u32)irq12, 0x08, 0x8e);
	idt_set_gate(45, (u32)irq13, 0x08, 0x8e);
	idt_set_gate(46, (u32)irq14, 0x08, 0x8e);
	idt_set_gate(47, (u32)irq15, 0x08, 0x8e);
}

/**
 * ISR
 */

PROTECTED static void (*isr_routines[256])(struct regs *) = { 0 };

PROTECTED const char *isr_exceptions[32] = {
	"Division By Zero",
	"Debug",
	"Non Maskable Interrupt",
	"Breakpoint",
	"Into Detected Overflow",
	"Out of Bounds",
	"Invalid Opcode",
	"No Coprocessor",

	"Double Fault",
	"Coprocessor Segment Overrun",
	"Bad TSS",
	"Segment Not Present",
	"Stack Fault",
	"General Protection Fault",
	"Page Fault",
	"Unknown Interrupt",

	"Coprocessor Fault",
	"Alignment Check",
	"Machine Check",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",

	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
};

CLEAR void isr_install_handler(int isr, void (*handler)(struct regs *r))
{
	isr_routines[isr] = handler;
}

CLEAR void isr_uninstall_handler(int isr)
{
	isr_routines[isr] = 0;
}

void isr_panic(struct regs *r)
{
	printf("%s Exception (code %x) at 0x%x (ring %d), exiting!\n", isr_exceptions[r->int_no],
	       r->err_code, r->eip, RING(r));
	struct proc *proc = proc_current();
	if (proc) {
		printf("\t-> Exception occurred in %s at addr 0x%x (offset 0x%x)\n", proc->name,
		       r->eip, r->eip - proc->entry);
		printf("\t\t-> Process: [entry: %x, kstack: %x, ustack: %x]\n", proc->entry,
		       proc->stack.kernel, proc->stack.user);
		proc_exit(proc, r, 1);
	} else {
		__asm__ volatile("cli\nhlt");
	}
	proc_yield_regs(r);
}

void isr_handler(struct regs *r);
void isr_handler(struct regs *r)
{
	assert(r->int_no < sizeof(isr_routines));

	// Execute fault handler if exists
	void (*handler)(struct regs * r) = isr_routines[r->int_no];
	if (handler)
		handler(r);
	else
		isr_panic(r);
}

CLEAR static void isr_install(void)
{
	idt_set_gate(0, (u32)isr0, 0x08, 0x8e);
	idt_set_gate(1, (u32)isr1, 0x08, 0x8e);
	idt_set_gate(2, (u32)isr2, 0x08, 0x8e);
	idt_set_gate(3, (u32)isr3, 0x08, 0x8e);
	idt_set_gate(4, (u32)isr4, 0x08, 0x8e);
	idt_set_gate(5, (u32)isr5, 0x08, 0x8e);
	idt_set_gate(6, (u32)isr6, 0x08, 0x8e);
	idt_set_gate(7, (u32)isr7, 0x08, 0x8e);

	idt_set_gate(8, (u32)isr8, 0x08, 0x8e);
	idt_set_gate(9, (u32)isr9, 0x08, 0x8e);
	idt_set_gate(10, (u32)isr10, 0x08, 0x8e);
	idt_set_gate(11, (u32)isr11, 0x08, 0x8e);
	idt_set_gate(12, (u32)isr12, 0x08, 0x8e);
	idt_set_gate(13, (u32)isr13, 0x08, 0x8e);
	idt_set_gate(14, (u32)isr14, 0x08, 0x8e);
	idt_set_gate(15, (u32)isr15, 0x08, 0x8e);

	idt_set_gate(16, (u32)isr16, 0x08, 0x8e);
	idt_set_gate(17, (u32)isr17, 0x08, 0x8e);
	idt_set_gate(18, (u32)isr18, 0x08, 0x8e);
	idt_set_gate(19, (u32)isr19, 0x08, 0x8e);
	idt_set_gate(20, (u32)isr20, 0x08, 0x8e);
	idt_set_gate(21, (u32)isr21, 0x08, 0x8e);
	idt_set_gate(22, (u32)isr22, 0x08, 0x8e);
	idt_set_gate(23, (u32)isr23, 0x08, 0x8e);

	idt_set_gate(24, (u32)isr24, 0x08, 0x8e);
	idt_set_gate(25, (u32)isr25, 0x08, 0x8e);
	idt_set_gate(26, (u32)isr26, 0x08, 0x8e);
	idt_set_gate(27, (u32)isr27, 0x08, 0x8e);
	idt_set_gate(28, (u32)isr28, 0x08, 0x8e);
	idt_set_gate(29, (u32)isr29, 0x08, 0x8e);
	idt_set_gate(30, (u32)isr30, 0x08, 0x8e);
	idt_set_gate(31, (u32)isr31, 0x08, 0x8e);

	// Set default routines
	for (u32 i = 0; i < 256; i++)
		isr_routines[i] = isr_panic;

	// Set page fault handler
	isr_install_handler(14, page_fault_handler);
}

/**
 * Combined
 */

CLEAR void interrupts_install(void)
{
	idt_install();
	isr_install();
	irq_install();
}
