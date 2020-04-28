#include <stdint.h>
#include <kernel/syscall/syscall.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/system.h>
#include <kernel/lib/stdio.h>
#include <kernel/io/io.h>

typedef uint32_t (*syscall_func)(uint32_t, ...);

uint32_t (*syscalls[])() = { [0] = (uint32_t(*)())halt_loop, // DEBUG!
			     [1] = (uint32_t(*)())sys_putch,
			     [2] = sys_getch,
			     [3] = sys_malloc,
			     [4] = sys_free };

void syscall_handler(struct regs *r)
{
	sti();
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