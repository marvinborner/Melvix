#include <stdint.h>
#include <kernel/syscall/syscall.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/io/io.h>
#include <kernel/paging/paging.h>
#include <kernel/system.h>

typedef uint32_t (*syscall_func)(unsigned int, ...);

uint32_t (*syscalls[])() = {
        [0] = (uint32_t (*)()) halt_loop, // DEBUG!
        [1] = sys_write,
        [2] = sys_read,
        [3] = (uint32_t (*)()) sys_writec,
        [4] = sys_readc,
        [5] = sys_get_pointers,
        [6] = sys_paging_alloc,
        [7] = sys_paging_free
};

void syscall_handler(struct regs *r)
{
    serial_write("Received syscall!\n");

    if (r->eax >= sizeof(syscalls) / sizeof(*syscalls))
        return;

    syscall_func location = (syscall_func) syscalls[r->eax];
    if (!location)
        return;

    r->eax = location(r->ebx, r->ecx, r->edx, r->esi, r->edi);
}

void syscalls_install()
{
    isr_install_handler(0x80, syscall_handler);
}