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

// External handlers (ASM)

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr128();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();
extern void irq128();

#endif
