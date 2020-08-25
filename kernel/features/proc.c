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
struct proc *kernel_proc;
struct list *proc_list;
struct node *current;

// TODO: Use less memcpy and only copy relevant registers
void scheduler(struct regs *regs)
{
	timer_handler();

	if (quantum == 0) {
		quantum = PROC_QUANTUM;
	} else {
		quantum--;
		return;
	}

	if (current && ((struct proc *)current->data)->state == PROC_RESOLVED) {
		memcpy(regs, &((struct proc *)current->data)->regs_backup, sizeof(struct regs));
		((struct proc *)current->data)->state = PROC_DEFAULT;
	}

	if (current)
		memcpy(&((struct proc *)current->data)->regs, regs, sizeof(struct regs));

	if (current && current->next)
		current = current->next;
	else
		current = proc_list->head;

	while (!current) {
		if (!current->next || !current->next->data) {
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

	/* printf("{%d}", ((struct proc *)current->data)->pid); */
}

void proc_print()
{
	struct node *node = proc_list->head;

	printf("\nPROCESSES\n");
	struct proc *proc;
	while (node && (proc = node->data)) {
		printf("Process %d: %s\n", proc->pid, proc->name);
		node = node->next;
	}
	printf("\n");
}

struct proc *proc_current()
{
	return current ? current->data : NULL;
}

void proc_send(struct proc *src, struct proc *dest, enum message_type type, void *data)
{
	// TODO: Use unique key instead of pid for IPC messaging
	assert(src && dest);
	struct proc_message *msg = malloc(sizeof(*msg));
	msg->src = src;
	msg->dest = dest;
	msg->msg = malloc(sizeof(struct message));
	msg->msg->src = src->pid;
	msg->msg->type = type;
	msg->msg->data = data;
	list_add(dest->messages, msg);
}

struct proc_message *proc_receive(struct proc *proc)
{
	if (proc->messages && proc->messages->head) {
		struct proc_message *msg = proc->messages->head->data;
		list_remove(proc->messages, proc->messages->head);
		return msg;
	} else {
		return NULL;
	}
}

struct proc *proc_from_pid(u32 pid)
{
	struct node *iterator = proc_list->head;
	do {
		if (((struct proc *)iterator->data)->pid == pid)
			return iterator->data;
	} while ((iterator = iterator->next) != NULL);
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

void proc_yield(struct regs *r)
{
	quantum = 0;
	scheduler(r);
}

struct proc *proc_make()
{
	struct proc *proc = malloc(sizeof(*proc));
	proc->pid = pid++;
	proc->messages = list_new();
	proc->state = PROC_DEFAULT;

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

	kernel_proc = proc_make();

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
