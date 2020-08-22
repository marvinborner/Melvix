// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <boot.h>
#include <cpu.h>
#include <interrupts.h>
#include <list.h>
#include <load.h>
#include <mem.h>
#include <print.h>
#include <proc.h>
#include <str.h>
#include <timer.h>

u32 pid = 0;
u32 quantum = 0;
struct list *proc_list;
struct node *current;

void scheduler(struct regs *regs)
{
	if (quantum == 0) {
		quantum = PROC_QUANTUM;
	} else {
		quantum--;
		return;
	}

	if (current)
		memcpy(&((struct proc *)current->data)->regs, regs, sizeof(struct regs));

	timer_handler();

	if (current && current->next)
		current = current->next;
	else
		current = proc_list->head;

	while (!current) {
		if (!current || !current->next || !current->next->data) {
			assert(proc_list->head);
			current = proc_list->head;
		} else {
			current = current->next;
		}
	}

	memcpy(regs, &((struct proc *)current->data)->regs, sizeof(struct regs));

	if (regs->cs != GDT_USER_CODE_OFFSET) {
		regs->gs = GDT_USER_DATA_OFFSET;
		regs->fs = GDT_USER_DATA_OFFSET;
		regs->es = GDT_USER_DATA_OFFSET;
		regs->ds = GDT_USER_DATA_OFFSET;
		regs->ss = GDT_USER_DATA_OFFSET;
		regs->cs = GDT_USER_CODE_OFFSET;
		regs->eflags = EFLAGS_ALWAYS | EFLAGS_INTERRUPTS;
	}

	struct proc *proc = (struct proc *)current->data;
	if (!proc->events || !proc->events->head)
		return;

	struct node *iterator = proc->events->head;
	do {
		struct proc_event *proc_event = iterator->data;
		printf("Event %d for pid %d\n", proc_event->desc->id, proc->pid);
		list_remove(proc->events, iterator);
	} while ((iterator = iterator->next) != NULL);

	/* printf("{%d}", ((struct proc *)current->data)->pid); */
}

void proc_print()
{
	struct node *node = proc_list->head;

	printf("\nPROCESSES\n");
	struct proc *proc;
	while (node && (proc = ((struct proc *)node->data))) {
		printf("Process %d: %s\n", proc->pid, proc->name);
		node = node->next;
	}
	printf("\n");
}

struct proc *proc_current()
{
	if (current)
		return (struct proc *)current->data;
	else
		return NULL;
}

void proc_exit(struct proc *proc, int status)
{
	assert(proc);
	printf("Process %d exited with status %d\n", proc->pid, status);

	struct node *iterator = proc_list->head;
	do {
		if (iterator->data == proc) {
			list_remove(proc_list, iterator);
			break;
		}
	} while ((iterator = iterator->next) != NULL);

	quantum = 0; // TODO: Add quantum to each process struct?
	sti();
	hlt();
}

struct proc *proc_make()
{
	struct proc *proc = malloc(sizeof(*proc));
	proc->pid = pid++;
	proc->events = list_new();

	if (current)
		list_add(proc_list, proc);

	return proc;
}

extern void proc_jump_userspace();

u32 _esp, _eip;
void proc_init()
{
	if (proc_list)
		return;

	cli();
	irq_install_handler(0, scheduler);
	proc_list = list_new();

	struct node *new = list_add(proc_list, proc_make());
	bin_load("/init", new->data);

	_eip = ((struct proc *)new->data)->regs.eip;
	_esp = ((struct proc *)new->data)->regs.useresp;

	int argc = 2;
	char **argv = malloc(sizeof(*argv) * (argc + 1));
	argv[0] = "init";
	argv[1] = (char *)boot_passed->vbe;
	argv[2] = NULL;

	((u32 *)_esp)[0] = argc; // First argument (argc)
	((u32 *)_esp)[1] = (u32)argv; // Second argument (argv)

	proc_jump_userspace();
	while (1) {
	};
}
