#ifndef MELVIX_INTERRUPTS_H
#define MELVIX_INTERRUPTS_H

#include <stdint.h>
#include <stddef.h>

/**
 * Initialize the Interrupt Descriptor Table with 256 entries
 */
void idt_install();

/**
 * Add new gate (Interrupt Service Routine) to the Interrupt Descriptor Table
 * @param num The index of the routine in the IDT
 * @param base The base address of the ISR
 * @param sel The kernel code segment (0x08)
 * @param flags The IDT access byte entry (P DPL 01110)
 */
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);

/**
 * Registers that get passed into an IRQ handler
 */
struct regs {
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags, useresp, ss;
};

/**
 * Install 32 exception ISRs into the IDT
 */
void isrs_install();

/**
 * Add a new Interrupt Request Handler
 * @param irq The index of the IRQ routine
 * @param handler The interrupt handler function
 */
typedef void (*irq_handler_t)(struct regs *);

void isr_install_handler(size_t isr, irq_handler_t handler);

/**
 * Uninstall a handler by index
 * @param irq The index of the IRQ routine that should be removed
 */
void isr_uninstall_handler(size_t isr);

/**
 * Initialize the Interrupt Requests by mapping the ISRs to the correct
 * entries in the IDT (install the exception handlers)
 */
void irq_install();

/**
 * Add a new Interrupt Request Handler
 * @param irq The index of the IRQ routine
 * @param handler The interrupt handler function
 */
void irq_install_handler(int irq, void (*handler)(struct regs *r));

/**
 * Uninstall a handler by index
 * @param irq The index of the IRQ routine that should be removed
 */
void irq_uninstall_handler(int irq);

/**
 * Execute the handler of the IRQ
 * @param r The ISR that should be handled
 */
void irq_handler(struct regs *r);

/**
 * Check if an IRQ is installed
 * @param irq The index of the IRQ routine that should be checked
 * @return 1 if installed, 0 if not
 */
int irq_is_installed(int irq);

// Defined in isr.asm
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

#endif