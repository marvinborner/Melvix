#include <stdint.h>
#include <kernel/syscall/syscall.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/graphics/vesa.h>
#include <kernel/io/io.h>

DEFN_SYSCALL1(vesa_draw_string, 0, const char *);

DEFN_SYSCALL1(vesa_draw_number, 1, int);

DEFN_SYSCALL1(serial_write, 2, const char *);

static void *syscalls[3] = {
        &vesa_draw_string,
        &vesa_draw_number,
        &serial_write,
};
uint32_t num_syscalls = 3;

void syscall_handler(struct regs *r) {
    serial_write("SYSCALL");
    if (r->eax >= num_syscalls)
        return;

    void *location = syscalls[r->eax];

    int ret;
    asm volatile (" \
     push %1; \
     push %2; \
     push %3; \
     push %4; \
     push %5; \
     call *%6; \
     pop %%ebx; \
     pop %%ebx; \
     pop %%ebx; \
     pop %%ebx; \
     pop %%ebx; \
   " : "=a" (ret) : "r" (r->edi), "r" (r->esi), "r" (r->edx), "r" (r->ecx), "r" (r->ebx), "r" (location));
    r->eax = ret;
}

void syscalls_install() {
    irq_install_handler(0x80, syscall_handler);
}
