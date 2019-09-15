#ifndef MELVIX_INTERRUPTS_H
#define MELVIX_INTERRUPTS_H

// IDT
void idt_install();

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);

// ISRS
void isrs_install();

struct regs {
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags, useresp, ss;
};

// IRQ
void irq_install();

void irq_install_handler(int irq, void (*handler)(struct regs *r));

void irq_uninstall_handler(int irq);

void irq_handler(struct regs *r);

#endif
