#include "../io/io.h"
#include "interrupts.h"

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

// Array to handle custom IRQ handlers
void *irq_routines[16] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
};

// Install custom IRQ handler
void irq_install_handler(int irq, void (*handler)(struct regs *r)) {
    irq_routines[irq] = handler;
}

// Removes the custom IRQ handler
void irq_uninstall_handler(int irq) {
    irq_routines[irq] = 0;
}

int irq_is_installed(int irq) {
    return irq_routines[irq] != 0;
}

// Remap IRQs for protected mode compatibility via the PIC
void irq_remap(void) {
    send_b(0x20, 0x11);
    send_b(0xA0, 0x11);
    send_b(0x21, 0x20);
    send_b(0xA1, 0x28);
    send_b(0x21, 0x04);
    send_b(0xA1, 0x02);
    send_b(0x21, 0x01);
    send_b(0xA1, 0x01);
    send_b(0x21, 0x0);
    send_b(0xA1, 0x0);
}

// Map ISRs to the correct entries in the IDT
void irq_install() {
    irq_remap();
    idt_set_gate(32, (unsigned) irq0, 0x08, 0x8E);
    idt_set_gate(33, (unsigned) irq1, 0x08, 0x8E);
    idt_set_gate(34, (unsigned) irq2, 0x08, 0x8E);
    idt_set_gate(35, (unsigned) irq3, 0x08, 0x8E);
    idt_set_gate(36, (unsigned) irq4, 0x08, 0x8E);
    idt_set_gate(37, (unsigned) irq5, 0x08, 0x8E);
    idt_set_gate(38, (unsigned) irq6, 0x08, 0x8E);
    idt_set_gate(39, (unsigned) irq7, 0x08, 0x8E);
    idt_set_gate(40, (unsigned) irq8, 0x08, 0x8E);
    idt_set_gate(41, (unsigned) irq9, 0x08, 0x8E);
    idt_set_gate(42, (unsigned) irq10, 0x08, 0x8E);
    idt_set_gate(43, (unsigned) irq11, 0x08, 0x8E);
    idt_set_gate(44, (unsigned) irq12, 0x08, 0x8E);
    idt_set_gate(45, (unsigned) irq13, 0x08, 0x8E);
    idt_set_gate(46, (unsigned) irq14, 0x08, 0x8E);
    idt_set_gate(47, (unsigned) irq15, 0x08, 0x8E);
}

// Handle IRQ ISRs
void irq_handler(struct regs *r) {
    void (*handler)(struct regs *r);

    // Execute custom handler if exists
    handler = irq_routines[r->int_no - 32];
    if (handler) {
        handler(r);
    }

    // Send end of interrupt to second (slave) IRQ controller
    if (r->int_no >= 40) {
        send_b(0xA0, 0x20);
    }

    // Send end of interrupt to master interrupt controller
    send_b(0x20, 0x20);
}
