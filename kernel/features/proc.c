// MIT License, Copyright (c) 2020 Marvin Borner

#include <cpu.h>
#include <interrupts.h>
#include <load.h>
#include <mem.h>
#include <print.h>
#include <proc.h>
#include <str.h>
#include <timer.h>

u32 pid = 0;
struct proc *root;
struct proc *current;
struct proc *last;

void scheduler(struct regs *regs)
{
	if (current)
		memcpy(&current->regs, regs, sizeof(struct regs));

	timer_handler();

	if (current && current->next)
		current = current->next;
	else
		current = root;

	while (current->state == PROC_ASLEEP)
		if (!current->next)
			current = root;
		else
			current = current->next;

	/* proc_print(); */
	memcpy(regs, &current->regs, sizeof(struct regs));

	if (regs->cs != GDT_USER_CODE_OFFSET) {
		regs->gs = GDT_USER_DATA_OFFSET;
		regs->fs = GDT_USER_DATA_OFFSET;
		regs->es = GDT_USER_DATA_OFFSET;
		regs->ds = GDT_USER_DATA_OFFSET;
		regs->ss = GDT_USER_DATA_OFFSET;
		regs->cs = GDT_USER_CODE_OFFSET;
		regs->eflags = EFLAGS_ALWAYS | EFLAGS_INTERRUPTS;
	}
	printf("%d", current->pid);
}

void proc_print()
{
	struct proc *proc = root;

	printf("\n");
	while (proc) {
		printf("Process %d [%s]: %s\n", proc->pid,
		       proc->state == PROC_RUNNING ? "running" : "sleeping", proc->name);
		proc = proc->next;
	}
	printf("\n");
}

void proc_attach(struct proc *proc)
{
	if (!last->next) {
		last->next = proc;
	} else {
		struct proc *save = last;
		while (save->next)
			save = save->next;
		save->next = proc;
	}
}

struct proc *proc_make()
{
	struct proc *proc = malloc(sizeof(*proc));
	proc->pid = pid++;
	proc->state = PROC_RUNNING;
	proc->next = NULL;

	if (current)
		proc_attach(proc);
	last = proc;
	return proc;
}

extern void proc_jump_userspace();

u32 _esp, _eip;
void proc_init()
{
	cli();
	irq_install_handler(0, scheduler);

	root = proc_make();
	elf_load("/init", root);
	proc_print();

	_eip = root->regs.eip;
	_esp = root->regs.esp;
	proc_jump_userspace();
}