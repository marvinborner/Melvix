#include <stdint.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/system.h>
#include <kernel/lib/string.h>
#include <kernel/lib/stdio.h>
#include <kernel/graphics/vesa.h>

// Install ISRs in IDT
void isrs_install()
{
    idt_set_gate(0, (unsigned) isr0, 0x08, 0x8E);
    idt_set_gate(1, (unsigned) isr1, 0x08, 0x8E);
    idt_set_gate(2, (unsigned) isr2, 0x08, 0x8E);
    idt_set_gate(3, (unsigned) isr3, 0x08, 0x8E);
    idt_set_gate(4, (unsigned) isr4, 0x08, 0x8E);
    idt_set_gate(5, (unsigned) isr5, 0x08, 0x8E);
    idt_set_gate(6, (unsigned) isr6, 0x08, 0x8E);
    idt_set_gate(7, (unsigned) isr7, 0x08, 0x8E);

    idt_set_gate(8, (unsigned) isr8, 0x08, 0x8E);
    idt_set_gate(9, (unsigned) isr9, 0x08, 0x8E);
    idt_set_gate(10, (unsigned) isr10, 0x08, 0x8E);
    idt_set_gate(11, (unsigned) isr11, 0x08, 0x8E);
    idt_set_gate(12, (unsigned) isr12, 0x08, 0x8E);
    idt_set_gate(13, (unsigned) isr13, 0x08, 0x8E);
    idt_set_gate(14, (unsigned) isr14, 0x08, 0x8E);
    idt_set_gate(15, (unsigned) isr15, 0x08, 0x8E);

    idt_set_gate(16, (unsigned) isr16, 0x08, 0x8E);
    idt_set_gate(17, (unsigned) isr17, 0x08, 0x8E);
    idt_set_gate(18, (unsigned) isr18, 0x08, 0x8E);
    idt_set_gate(19, (unsigned) isr19, 0x08, 0x8E);
    idt_set_gate(20, (unsigned) isr20, 0x08, 0x8E);
    idt_set_gate(21, (unsigned) isr21, 0x08, 0x8E);
    idt_set_gate(22, (unsigned) isr22, 0x08, 0x8E);
    idt_set_gate(23, (unsigned) isr23, 0x08, 0x8E);

    idt_set_gate(24, (unsigned) isr24, 0x08, 0x8E);
    idt_set_gate(25, (unsigned) isr25, 0x08, 0x8E);
    idt_set_gate(26, (unsigned) isr26, 0x08, 0x8E);
    idt_set_gate(27, (unsigned) isr27, 0x08, 0x8E);
    idt_set_gate(28, (unsigned) isr28, 0x08, 0x8E);
    idt_set_gate(29, (unsigned) isr29, 0x08, 0x8E);
    idt_set_gate(30, (unsigned) isr30, 0x08, 0x8E);
    idt_set_gate(31, (unsigned) isr31, 0x08, 0x8E);

    idt_set_gate(0x80, (unsigned) isr128, 0x08, 0xEE);

    vga_log("Installed Interrupt Service Routines");
}

irq_handler_t isr_routines[256] = {0};

// Install custom IRQ handler
void isr_install_handler(size_t isr, irq_handler_t handler)
{
    isr_routines[isr] = handler;
}

// Removes the custom IRQ handler
void isr_uninstall_handler(size_t isr)
{
    isr_routines[isr] = 0;
}

// Error exception messages
const char *exception_messages[] = {
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
        "Reserved"
};

// Master exception/interrupt/fault handler - halt via panic
void fault_handler(struct regs *r)
{
    irq_handler_t handler = isr_routines[r->int_no];
    if (handler) {
        handler(r);
    } else {
        uint32_t faulting_address;
        asm ("mov %%cr2, %0" : "=r" (faulting_address));

        serial_printf(
                "\n[DEBUG]\nEIP: 0x%x\nEAX: 0x%x\nEBX: 0x%x\nECX: 0x%x\nEDX: 0x%x\nESP: 0x%x\nFault addr: 0x%x\nErr flag: 0x%x\nErr code: 0x%x\nINT code: 0x%x\nINT msg: %s",
                r->eip, r->eax, r->ebx, r->ecx, r->edx, r->esp, faulting_address, r->eflags, r->err_code, r->int_no,
                exception_messages[r->int_no]
        );
        // halt_loop(); // Idk loop?
        char *message = (char *) exception_messages[r->int_no];
        strcat(message, " Exception");

        // Show message if there wasn't an error in video memory
        if (faulting_address != (uint32_t) fb)
            panic(message);
        else
            halt_loop();
    }
}