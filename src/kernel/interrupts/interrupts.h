#ifndef MELVIX_INTERRUPTS_H
#define MELVIX_INTERRUPTS_H

#include <stdint.h>

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
 * Install 32 exception ISRs into the IDT
 */
void isrs_install();

/**
 * Ignore interrupt
 */
void isr_ignore(uint8_t int_no);

/**
 * Un-ignore interrupt
 */
void isr_remember(uint8_t int_no);

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

#endif
