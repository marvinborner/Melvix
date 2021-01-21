// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <boot.h>
#include <cpu.h>
#include <fs.h>
#include <load.h>
#include <mem.h>
#include <print.h>
#include <proc.h>
#include <stack.h>
#include <str.h>
#include <timer.h>

u32 current_pid = 0;
u32 quantum = 0;
struct proc *priority_proc;
struct list *proc_list;
struct node *idle_proc;
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
		current = idle_proc;
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

void kernel_idle()
{
	while (1)
		;
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

void proc_clear_quantum()
{
	quantum = 0;
}

void proc_exit(struct proc *proc, int status)
{
	assert(proc);

	int res = 0;
	struct node *iterator = proc_list->head;
	while (iterator) {
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

	proc_clear_quantum(); // TODO: Add quantum to each process struct?
	sti();
	hlt();
}

void proc_yield(struct regs *r)
{
	proc_clear_quantum();
	scheduler(r);
}

void proc_enable_waiting(u32 dev_id)
{
	struct node *iterator = proc_list->head;
	while (iterator) {
		struct proc *p = iterator->data;
		struct proc_wait *w = &p->wait;
		if (p && w && w->id == dev_id) {
			struct regs *r = &p->regs;
			r->eax = (u32)w->func((char *)r->ebx, (void *)r->ecx, r->edx, r->esi);
			memset(&p->wait, 0, sizeof(p->wait));
			p->state = PROC_RUNNING;
		}
		iterator = iterator->next;
	}
}

struct proc *proc_make(void)
{
	struct proc *proc = malloc(sizeof(*proc));
	proc->pid = current_pid++;
	proc->messages = stack_new();
	proc->state = PROC_RUNNING;

	if (current)
		list_add(proc_list, proc);

	return proc;
}

u32 procfs_write(const char *path, void *buf, u32 offset, u32 count, struct device *dev)
{
	while (*path == '/')
		path++;

	int pid = 0;
	while (path[0] >= '0' && path[0] <= '9') {
		pid = pid * 10 + (path[0] - '0');
		path++;
	}

	if (!pid && !memcmp(path, "self/", 5)) {
		pid = proc_current()->pid;
		path += 4;
	}

	if (pid) {
		struct proc *p = proc_from_pid(pid);
		if (!p || path[0] != '/')
			return 0;

		path++;
		if (!memcmp(path, "msg", 4)) {
			stack_push_bot(p->messages, buf);
			return count;
		}
	}

	printf("%s - off: %d, cnt: %d, buf: %x, dev %x\n", path, offset, count, buf, dev);
	return 0;
}

u32 procfs_read(const char *path, void *buf, u32 offset, u32 count, struct device *dev)
{
	(void)dev;

	while (*path == '/')
		path++;

	int pid = 0;
	while (path[0] >= '0' && path[0] <= '9') {
		pid = pid * 10 + (path[0] - '0');
		path++;
	}

	if (!pid && !memcmp(path, "self/", 5)) {
		pid = proc_current()->pid;
		path += 4;
	}

	if (pid) {
		struct proc *p = proc_from_pid(pid);
		if (!p || path[0] != '/')
			return 0;

		path++;
		if (!memcmp(path, "pid", 4)) {
			//memcpy(buf, ((u8 *)p->pid) + offset, count);
			*(u32 *)buf = p->pid;
			return count;
		} else if (!memcmp(path, "name", 5)) {
			memcpy(buf, p->name + offset, count);
			return count;
		} else if (!memcmp(path, "status", 7)) {
			const char *status = p->state == PROC_RUNNING ? "running" : "sleeping";
			memcpy(buf, status + offset, count);
			return count;
		} else if (!memcmp(path, "msg", 4)) {
			if (stack_empty(p->messages)) {
				return 0;
			} else {
				u8 *msg = stack_pop(p->messages);
				memcpy(buf, msg + offset, count);
				return count;
			}
		}
	}

	return 0;
}

u32 procfs_ready(const char *path, struct device *dev)
{
	(void)path;
	(void)dev;
	return 1;
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

	// Procfs
	struct vfs *vfs = malloc(sizeof(*vfs));
	vfs->type = VFS_PROCFS;
	vfs->read = procfs_read;
	vfs->write = procfs_write;
	vfs->ready = procfs_ready;
	struct device *dev = malloc(sizeof(*dev));
	dev->name = "proc";
	dev->type = DEV_CHAR;
	dev->vfs = vfs;
	device_add(dev);
	vfs_mount(dev, "/proc/");

	// Idle proc
	struct proc *kernel_proc = proc_make();
	void (*func)() = kernel_idle;
	proc_load(kernel_proc, *(void **)&func);
	strcpy(kernel_proc->name, "idle");
	kernel_proc->state = PROC_SLEEPING;
	idle_proc = list_add(proc_list, kernel_proc);

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
