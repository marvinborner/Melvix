#include <stdint.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/memory/paging.h>
#include <kernel/tasks/process.h>
#include <kernel/lib/lib.h>
#include <kernel/system.h>

uint32_t sys_fork(struct regs *r)
{
	struct page_directory *dir = paging_copy_user_directory(current_proc->cr3);
	struct process *proc = process_make_new();
	proc->cr3 = dir;
	memcpy(&proc->registers, r, sizeof(struct regs));
	proc->registers.eax = proc->pid;
	proc->pid = current_proc->pid;

	process_spawn(proc);

	return 0;
}