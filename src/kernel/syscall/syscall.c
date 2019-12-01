#include <stdint.h>
#include <kernel/syscall/syscall.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/io/io.h>

void syscalls_install()
{
    // 11100111
    idt_set_gate(0x80, (unsigned) idt_syscall, 0x08, 0xEE);
}

uint32_t syscall_handler(uint32_t id, uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    serial_write("Received syscall!\n");

    switch (id) {
        case 1:
            return sys_write((char *) arg0, arg1);
    }

    return -1;
}