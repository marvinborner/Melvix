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

u32 current_pid = 0;
u32 quantum = 0;
struct proc *priority_proc;
struct list *proc_list;
struct node *current;

// TODO: Use less memcpy and only copy relevant registers
// TODO: 20 priority queues (https://www.kernel.org/doc/html/latest/scheduler/sched-nice-design.html)
void scheduler(struct regs *regs)
{
	if (quantum == 0) {
		quantum = PROC_QUANTUM;
	} else {
		quantum--;
		return;
	}

	assert(proc_list->head);

	if (current)
		memcpy(&((struct proc *)current->data)->regs, regs, sizeof(struct regs));

	if (priority_proc && priority_proc->state == PROC_RUNNING) {
		current = list_first_data(proc_list, priority_proc);
		priority_proc = NULL;
		assert(current);
	} else if (current && current->next &&
		   ((struct proc *)current->next->data)->state == PROC_RUNNING) {
		current = current->next;
	} else if (((struct proc *)proc_list->head->data)->state == PROC_RUNNING) {
		current = proc_list->head;
	} else {
		print("TODO: All processes are sleeping!\n"); // TODO!
		/* loop(); */
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

void proc_print(void)
{
	struct node *node = proc_list->head;

	printf("\nPROCESSES\n");
	struct proc *proc = NULL;
	while (node && (proc = node->data)) {
		printf("Process %d: %s [%s]\n", proc->pid, proc->name,
		       proc->state == PROC_RUNNING ? "RUNNING" : "SLEEPING");
		node = node->next;
	}
	printf("\n");
}

struct proc *proc_current(void)
{
	return current && current->data ? current->data : NULL;
}

void proc_send(struct proc *src, struct proc *dest, u32 type, void *data)
{
	// TODO: Use unique key instead of pid for IPC
	if (!src || !dest)
		return;
	struct proc_message *msg = malloc(sizeof(*msg));
	msg->src = src;
	msg->dest = dest;
	msg->msg = malloc(sizeof(*msg->msg));
	msg->msg->src = (int)src->pid;
	msg->msg->type = (int)type;
	msg->msg->data = data;
	list_add(dest->messages, msg);
	priority_proc = dest;
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
	while (iterator != NULL) {
		if (((struct proc *)iterator->data)->pid == pid)
			return iterator->data;
		iterator = iterator->next;
	}
	return NULL;
}

void proc_exit(struct proc *proc, int status)
{
	assert(proc);

	int res = 0;
	struct node *iterator = proc_list->head;
	while (iterator != NULL) {
		if (iterator->data == proc) {
			res = 1;
			list_remove(proc_list, iterator);
			break;
		}
		iterator = iterator->next;
	}

	if (memcmp(proc, current->data, sizeof(*proc)) == 0)
		current = NULL;

	if (res)
		printf("Process %s exited with status %d\n", proc->name, status);

	quantum = 0; // TODO: Add quantum to each process struct?
	sti();
	hlt();
}

void proc_yield(struct regs *r)
{
	quantum = 0;
	scheduler(r);
}

struct proc *proc_make(void)
{
	struct proc *proc = malloc(sizeof(*proc));
	proc->pid = current_pid++;
	proc->messages = list_new();
	proc->state = PROC_RUNNING;

	if (current)
		list_add(proc_list, proc);

	return proc;
}

extern void proc_jump_userspace(void);

u32 _esp, _eip;
void proc_init(void)
{
	if (proc_list)
		return;

	cli();
	scheduler_enable();
	proc_list = list_new();

	kernel_proc = proc_make();

	struct node *new = list_add(proc_list, proc_make());
	bin_load((char *)"/bin/init", new->data);
	current = new;

	_eip = ((struct proc *)new->data)->regs.eip;
	_esp = ((struct proc *)new->data)->regs.useresp;

	u32 argc = 2;
	char **argv = malloc(sizeof(*argv) * (argc + 1));
	argv[0] = strdup("init");
	argv[1] = (char *)boot_passed->vbe;
	argv[2] = NULL;

	((u32 *)_esp)[0] = argc; // First argument (argc)
	((u32 *)_esp)[1] = (u32)argv; // Second argument (argv)

	proc_jump_userspace();
	while (1) {
	};
}
