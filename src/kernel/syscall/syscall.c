#include <stdint.h>
#include <kernel/syscall/syscall.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/system.h>
#include <kernel/lib/stdio.h>
#include <kernel/io/io.h>
#include <kernel/tasks/process.h>

typedef uint32_t (*syscall_func)(uint32_t, ...);

uint32_t (*syscalls[])() = { [0] = (uint32_t(*)())halt_loop, // DEBUG!
			     [1] = sys_exec,
			     [2] = (uint32_t(*)())sys_putch,
			     [3] = sys_getch,
			     [4] = sys_malloc,
			     [5] = sys_free,
			     [6] = sys_pointers };

void syscall_handler(struct regs *r)
{
	cli();

	if (r->eax >= sizeof(syscalls) / sizeof(*syscalls))
		return;

	syscall_func location = (syscall_func)syscalls[r->eax];
	if (!location)
		return;

	log("[SYSCALL] %s called %d with 0x%x 0x%x 0x%x 0x%x 0x%x", current_proc->name, r->eax,
	    location, r->ebx, r->ecx, r->edx, r->esi, r->edi);

	r->eax = location(r->ebx, r->ecx, r->edx, r->esi, r->edi);

	sti();
}

void syscalls_install()
{
	isr_install_handler(0x80, syscall_handler);
}