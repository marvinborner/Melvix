#include <stdint.h>
#include <kernel/syscall/syscall.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/io/io.h>

typedef uint32_t (*syscall_func)(unsigned int, ...);

void syscall_handler(struct regs *r)
{
    serial_write("Received syscall!\n");
    serial_write_dec(r->eax);
    serial_write("\n");
    serial_write_dec(r->ecx);
    syscall_func location = (syscall_func) sys_write;
    location(r->ebx, r->ecx, r->edx, r->esi, r->edi);
}

void syscalls_install()
{
    isr_install_handler(0x80, syscall_handler);
}