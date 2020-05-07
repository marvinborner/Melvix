#include <interrupts/interrupts.h>
#include <io/io.h>
#include <lib/lib.h>
#include <memory/paging.h>
#include <stdint.h>
#include <system.h>
#include <tasks/process.h>

u32 sys_fork(struct regs *r)
{
	cli();
	struct page_directory *dir = paging_copy_user_directory(current_proc->cr3);
	struct process *proc = process_make_new();
	proc->cr3 = dir;
	memcpy(&proc->registers, r, sizeof(struct regs));
	proc->registers.eax = proc->pid;
	proc->pid = current_proc->pid + 1;

	sti();
	process_spawn(proc);

	return 0;
}