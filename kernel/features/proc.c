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
struct list *proc_list;
struct node *current;

void scheduler(struct regs *regs)
{
	if (current)
		memcpy(&((struct proc *)current->data)->regs, regs, sizeof(struct regs));

	timer_handler();

	if (current && current->next)
		current = current->next;
	else
		current = proc_list->head;

	while (!current || ((struct proc *)current->data)->state == PROC_ASLEEP) {
		if (!current || !current->next || !current->next->data) {
			assert(proc_list->head);
			assert(((struct proc *)proc_list->head->data)->state != PROC_ASLEEP);
			current = proc_list->head;
		} else {
			current = current->next;
		}
	}

	/* proc_print(); */
	memcpy(regs, &((struct proc *)current->data)->regs, sizeof(struct regs));

	if (((struct proc *)current->data)->event) {
		struct proc *proc = (struct proc *)current->data;
		// TODO: Modify and backup EIP
		printf("Event %d for pid %d\n", proc->event, proc->pid);
		// TODO: Clear bit after resolve
		proc->event = 0;
	}

	if (regs->cs != GDT_USER_CODE_OFFSET) {
		regs->gs = GDT_USER_DATA_OFFSET;
		regs->fs = GDT_USER_DATA_OFFSET;
		regs->es = GDT_USER_DATA_OFFSET;
		regs->ds = GDT_USER_DATA_OFFSET;
		regs->ss = GDT_USER_DATA_OFFSET;
		regs->cs = GDT_USER_CODE_OFFSET;
		regs->eflags = EFLAGS_ALWAYS | EFLAGS_INTERRUPTS;
	}
	/* printf("{%d}", ((struct proc *)current->data)->pid); */
}

void proc_print()
{
	struct node *node = proc_list->head;

	printf("\nPROCESSES\n");
	struct proc *proc;
	while (node && (proc = ((struct proc *)node->data))) {
		printf("Process %d [%s]: %s\n", proc->pid,
		       proc->state == PROC_RUNNING ? "running" : "sleeping", proc->name);
		node = node->next;
	}
	printf("\n");
}

struct proc *proc_current()
{
	return (struct proc *)current->data;
}

void proc_exit(struct proc *proc, int status)
{
	printf("Process %d exited with status %d\n", proc->pid, status);
	proc->state = status == 0 ? PROC_ASLEEP : PROC_ERROR;

	struct node *iterator = proc_list->head;
	do {
		if (iterator->data == proc) {
			list_remove(proc_list, iterator);
			break;
		}
	} while ((iterator = iterator->next) != NULL);
	proc_print();
}

struct proc *proc_make()
{
	struct proc *proc = malloc(sizeof(*proc));
	proc->pid = pid++;
	proc->state = PROC_RUNNING;

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
	proc_print();

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
