// MIT License, Copyright (c) 2020 Marvin Borner

#include <cpu.h>
#include <interrupts.h>
#include <load.h>
#include <mem.h>
#include <print.h>
#include <proc.h>
#include <timer.h>

u32 pid = 0;
struct proc *root;
struct proc *current;
struct proc *last;

void scheduler(struct regs *regs)
{
	if (current) {
		printf("%d", current->pid);
		memcpy(&current->regs, regs, sizeof(struct regs));
	}

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

	memcpy(regs, &current->regs, sizeof(struct regs));
}

void proc_print()
{
	struct proc *proc = root;

	printf("\n");
	while (proc) {
		printf("Process %d [%s]\n", proc->pid,
		       proc->state == PROC_RUNNING ? "running" : "sleeping");
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
	/* if (current) */
	/* 	current->state = PROC_ASLEEP; */
	struct proc *proc = malloc(sizeof(*proc));
	proc->pid = pid++;
	proc->state = PROC_RUNNING;
	if (current)
		proc_attach(proc);
	last = proc;
	return proc;
}

void proc_init()
{
	cli();
	irq_install_handler(0, scheduler);

	root = proc_make();
	bin_load("/init", root);
	proc_print();
	sti();
	hlt();
}
