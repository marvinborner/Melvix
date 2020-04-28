#include <stdint.h>
#include <kernel/syscall/syscall.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/system.h>
#include <kernel/lib/stdio.h>

typedef uint32_t (*syscall_func)(unsigned int, ...);

uint32_t (*syscalls[])() = { [0] = (uint32_t(*)())halt_loop, // DEBUG!
			     [1] = sys_write,
			     [2] = sys_read,
			     [3] = (uint32_t(*)())sys_writec,
			     [4] = sys_readc,
			     [5] = sys_get_pointers,
			     [6] = sys_alloc,
			     [7] = sys_free };

void syscall_handler(struct regs *r)
{
	log("Received syscall!");

	if (r->eax >= sizeof(syscalls) / sizeof(*syscalls))
		return;

	syscall_func location = (syscall_func)syscalls[r->eax];
	if (!location)
		return;

	log("[SYSCALL] %d (0x%x) 0x%x 0x%x 0x%x 0x%x 0x%x", r->eax, location, r->ebx, r->ecx,
	    r->edx, r->esi, r->edi);

	r->eax = location(r->ebx, r->ecx, r->edx, r->esi, r->edi);
}

void syscalls_install()
{
	isr_install_handler(0x80, syscall_handler);
}
