#include <stdint.h>
#include <stddef.h>
#include <kernel/system.h>
#include <kernel/tasks/userspace.h>
#include <kernel/tasks/process.h>
#include <kernel/memory/paging.h>
#include <kernel/io/io.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/lib/lib.h>

struct process *proc_bottom = NULL;

uint32_t hl_cr3;
uint32_t hl_eip;
uint32_t hl_esp;

uint32_t spawn_child(struct process *child)
{
	return (uint32_t)-1;
}

void userspace_enter(struct process *proc)
{
	proc_bottom = proc;
	proc->next = NULL;
	hl_eip = proc->registers.eip;
	hl_esp = proc->registers.esp;
	paging_switch_directory(proc->cr3);

	current_proc = proc;

	sti(); // TODO: Prevent race conditions in userspace jumping
	debug("Jumping to userspace!");
	jump_userspace();
}

void single_yield(struct process *proc, struct regs *regs)
{
	memcpy(&proc_bottom->registers, regs, sizeof(struct regs));

	if (proc == proc_bottom)
		panic("Can't return from parent process");

	proc->next = proc_bottom;
	proc_bottom = proc;

	memcpy(regs, &proc->registers, sizeof(struct regs));
	paging_switch_directory(proc->cr3);
}

uint32_t single_exit(struct regs *regs)
{
	//close(current_proc->stdout);
	//close(current_proc->stderr);

	uint32_t hold = regs->ebx;
	proc_bottom = proc_bottom->next;

	if (proc_bottom == NULL)
		panic("Return from process with no parent");

	memcpy(regs, &proc_bottom->registers, sizeof(struct regs));
	paging_switch_directory(proc_bottom->cr3);

	return hold;
}