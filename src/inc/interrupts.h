// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef IDT_H
#define IDT_H

#include <def.h>

struct regs {
	u32 gs, fs, es, ds;
	u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
	u32 int_no, err_code;
	u32 eip, cs, eflags, useresp, ss;
};

struct idt_entry {
	u16 base_low;
	u16 sel; // Kernel segment
	u8 always0; // Always 0
	u8 flags;
	u16 base_high;
} __attribute__((packed));

struct idt_ptr {
	u16 limit;
	void *base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idt_ptr;

void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags);

void irq_install_handler(int irq, void (*handler)(struct regs *r));
void irq_uninstall_handler(int irq);

void isr_install_handler(int isr, void (*handler)(struct regs *r));
void isr_uninstall_handler(int isr);

void interrupts_install();

#endif
