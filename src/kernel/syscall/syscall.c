#include <stdint.h>
#include <kernel/syscall/syscall.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/io/io.h>

typedef uint32_t (*syscall_func)(unsigned int, ...);

uint32_t (*syscalls[])() = {
        [1] = sys_write,
        [2] = sys_read
};

void syscall_handler(struct regs *r)
{
    serial_write("Received syscall!\n");

    if (r->eax >= sizeof(syscalls) / sizeof(*syscalls))
        return;

    syscall_func location = (syscall_func) syscalls[r->eax];
    if (!location)
        return;

    location(r->ebx, r->ecx, r->edx, r->esi, r->edi);
}

void syscalls_install()
{
    isr_install_handler(0x80, syscall_handler);
}