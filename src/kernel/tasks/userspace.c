#include <interrupts/interrupts.h>
#include <io/io.h>
#include <lib/lib.h>
#include <memory/paging.h>
#include <stddef.h>
#include <stdint.h>
#include <system.h>
#include <tasks/process.h>
#include <tasks/userspace.h>

struct process *proc_bottom = NULL;

u32 hl_cr3;
u32 hl_eip;
u32 hl_esp;

u32 spawn_child(struct process *child)
{
	return (u32)-1;
}

void userspace_enter(struct process *proc)
{
	cli();
	proc_bottom = proc;
	proc->next = NULL;
	hl_eip = proc->registers.eip;
	hl_esp = proc->registers.esp;
	paging_switch_directory(proc->cr3);

	current_proc = proc;

	debug("Jumping to userspace!");
	sti(); // TODO: Prevent race conditions in userspace jumping
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

u32 single_exit(struct regs *regs)
{
	u32 hold = regs->ebx;
	proc_bottom = proc_bottom->next;

	if (proc_bottom == NULL)
		panic("Return from process with no parent");

	memcpy(regs, &proc_bottom->registers, sizeof(struct regs));
	paging_switch_directory(proc_bottom->cr3);

	return hold;
}