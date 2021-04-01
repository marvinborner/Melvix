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
} PACKED;

struct idt_ptr {
	u16 limit;
	void *base;
} PACKED;

void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags);

void irq_install_handler(int irq, void (*handler)(struct regs *r)) NONNULL;
void irq_uninstall_handler(int irq);

void isr_install_handler(int isr, void (*handler)(struct regs *r)) NONNULL;
void isr_uninstall_handler(int isr);
void isr_panic(struct regs *r) NONNULL;

void interrupts_install(void);

// External handlers (ASM)

extern void isr0(struct regs *r);
extern void isr1(struct regs *r);
extern void isr2(struct regs *r);
extern void isr3(struct regs *r);
extern void isr4(struct regs *r);
extern void isr5(struct regs *r);
extern void isr6(struct regs *r);
extern void isr7(struct regs *r);
extern void isr8(struct regs *r);
extern void isr9(struct regs *r);
extern void isr10(struct regs *r);
extern void isr11(struct regs *r);
extern void isr12(struct regs *r);
extern void isr13(struct regs *r);
extern void isr14(struct regs *r);
extern void isr15(struct regs *r);
extern void isr16(struct regs *r);
extern void isr17(struct regs *r);
extern void isr18(struct regs *r);
extern void isr19(struct regs *r);
extern void isr20(struct regs *r);
extern void isr21(struct regs *r);
extern void isr22(struct regs *r);
extern void isr23(struct regs *r);
extern void isr24(struct regs *r);
extern void isr25(struct regs *r);
extern void isr26(struct regs *r);
extern void isr27(struct regs *r);
extern void isr28(struct regs *r);
extern void isr29(struct regs *r);
extern void isr30(struct regs *r);
extern void isr31(struct regs *r);
extern void isr128(struct regs *r);

extern void irq0(struct regs *r);
extern void irq1(struct regs *r);
extern void irq2(struct regs *r);
extern void irq3(struct regs *r);
extern void irq4(struct regs *r);
extern void irq5(struct regs *r);
extern void irq6(struct regs *r);
extern void irq7(struct regs *r);
extern void irq8(struct regs *r);
extern void irq9(struct regs *r);
extern void irq10(struct regs *r);
extern void irq11(struct regs *r);
extern void irq12(struct regs *r);
extern void irq13(struct regs *r);
extern void irq14(struct regs *r);
extern void irq15(struct regs *r);
extern void irq128(struct regs *r);

#endif
