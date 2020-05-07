#include <stdint.h>
#include <common.h>
#include <syscall/syscall.h>
#include <interrupts/interrupts.h>
#include <system.h>
#include <lib/stdio.h>
#include <io/io.h>
#include <tasks/process.h>

typedef u32 (*syscall_func)(u32, ...);

u32 (*syscalls[])() = { [SYS_HALT] = (u32(*)())halt_loop, // DEBUG!
			[SYS_EXIT] = sys_exit,
			[SYS_FORK] = sys_fork,
			[SYS_READ] = sys_read,
			[SYS_WRITE] = sys_write,
			[SYS_EXEC] = sys_exec,
			[SYS_WAIT] = sys_wait,
			[SYS_GET_PID] = sys_get_pid,
			[SYS_MALLOC] = sys_malloc,
			[SYS_FREE] = sys_free,
			[SYS_GET] = sys_get,
			[SYS_MAP] = sys_map };

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

	if (r->eax == SYS_FORK) // TODO: Fix hardcoded fork parameters
		r->eax = location(r);
	else
		r->eax = location(r->ebx, r->ecx, r->edx, r->esi, r->edi);
	sti();
}

void syscalls_install()
{
	isr_install_handler(0x80, syscall_handler);
}
