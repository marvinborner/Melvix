#include <stdint.h>
#include <kernel/syscall/syscall.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/system.h>
#include <kernel/lib/stdio.h>
#include <kernel/io/io.h>
#include <kernel/tasks/process.h>

typedef uint32_t (*syscall_func)(uint32_t, ...);

uint32_t (*syscalls[])() = { [0] = (uint32_t(*)())halt_loop, // DEBUG!
			     [1] = sys_exit,
			     [2] = sys_fork,
			     [3] = sys_read,
			     [4] = sys_write,
			     [5] = sys_exec,
			     [6] = sys_get_pid,
			     [7] = sys_malloc,
			     [8] = sys_free };

void syscall_handler(struct regs *r)
{
	cli();

	if (r->eax >= sizeof(syscalls) / sizeof(*syscalls))
		return;

	syscall_func location = (syscall_func)syscalls[r->eax];
	if (!location)
		return;

	log("[SYSCALL] %d at [0x%x] with 0x%x 0x%x 0x%x 0x%x", r->eax, location, r->ebx, r->ecx,
	    r->edx, r->esi, r->edi);

	if (r->eax == 2) // TODO: Fix hardcoded fork parameters
		r->eax = location(r);
	else
		r->eax = location(r->ebx, r->ecx, r->edx, r->esi, r->edi);

	sti();
}

void syscalls_install()
{
	isr_install_handler(0x80, syscall_handler);
}