// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <dev.h>
#include <drivers/cpu.h>
#include <drivers/gdt.h>
#include <errno.h>
#include <fs.h>
#include <load.h>
#include <mem.h>
#include <mm.h>
#include <print.h>
#include <proc.h>
#include <stack.h>
#include <str.h>
#include <timer.h>

#define PROC(node) ((struct proc *)node->data)

static u32 locked = 0;
static u32 current_pid = 0;
static struct node *current = NULL;
PROTECTED static struct node *idle_proc = NULL;

PROTECTED static struct list *proc_list_running = NULL;
PROTECTED static struct list *proc_list_blocked = NULL;
PROTECTED static struct list *proc_list_idle = NULL;

// TODO: 20 priority queues (https://www.kernel.org/doc/html/latest/scheduler/sched-nice-design.html)
HOT FLATTEN u32 scheduler(u32 esp)
{
	spinlock(&locked);

	if (!current) {
		current = idle_proc;
		locked = 0;
		return PROC(current)->stack.kernel_ptr;
	}

	if (PROC(current)->quantum.cnt >= PROC(current)->quantum.val) {
		PROC(current)->quantum.cnt = 0;
	} else {
		PROC(current)->quantum.cnt++;
		locked = 0;
		return esp;
	}

	fpu_save(PROC(current));
	PROC(current)->stack.kernel_ptr = esp;

	if (current->next) {
		current = current->next;
	} else if (proc_list_running->head) {
		current = proc_list_running->head;
	} else {
		current = idle_proc;
	}

	memory_switch_dir(PROC(current)->page_dir);
	tss_set_stack(PROC(current)->stack.kernel);
	fpu_restore(PROC(current));

#if DEBUG_SCHEDULER
	if (current != idle_proc) {
		struct int_frame_user *frame =
			(struct int_frame_user *)PROC(current)->stack.kernel_ptr;
		printf("%s (%d): eip %x esp %x useresp %x\n", PROC(current)->name,
		       PROC(current)->pid, frame->eip, frame->esp, frame->useresp);
	}
#endif

	locked = 0;
	return PROC(current)->stack.kernel_ptr;
}

void proc_print(void)
{
	struct node *node = proc_list_running->head;
	struct proc *proc = NULL;

	print("--- PROCESSES ---\n");
	while (node && (proc = node->data)) {
		printf("Process %d: %s [%s]\n", proc->pid, proc->name,
		       proc->state == PROC_RUNNING ? "RUNNING" : "SLEEPING");
		node = node->next;
	}
	print("\n");
}

struct proc *proc_current(void)
{
	return current && current->data ? current->data : NULL;
}

u8 proc_super(void)
{
	struct proc *proc = proc_current();
	if (proc)
		return proc->priv == PROC_PRIV_ROOT || proc->priv == PROC_PRIV_KERNEL;
	else if (current_pid == 0)
		return 1; // Kernel has super permissions
	else
		return 0; // Should never happen
}

u8 proc_idle(void)
{
	struct proc *proc = proc_current();
	if (proc)
		return proc == idle_proc->data;
	else
		return 0;
}

struct proc *proc_from_pid(u32 pid)
{
	struct node *iterator = NULL;

	iterator = proc_list_blocked->head;
	while (iterator) {
		if (PROC(iterator)->pid == pid)
			return iterator->data;
		iterator = iterator->next;
	}

	iterator = proc_list_running->head;
	while (iterator) {
		if (PROC(iterator)->pid == pid)
			return iterator->data;
		iterator = iterator->next;
	}

	return NULL;
}

void proc_timer_check(u32 time)
{
	struct node *iterator = proc_list_blocked->head;
	while (iterator) {
		struct proc *proc = PROC(iterator);
		if (proc->timer.mode == TIMER_MODE_SLEEP && time >= proc->timer.data)
			dev_unblock_pid(proc->pid);
		iterator = iterator->next;
	}
}

CLEAR void proc_set_quantum(struct proc *proc, u32 value)
{
	proc->quantum.val = value;
}

void proc_reset_quantum(struct proc *proc)
{
	proc->quantum.cnt = proc->quantum.val;
}

void proc_state(struct proc *proc, enum proc_state state)
{
	assert(proc != idle_proc->data);

	if (state == PROC_RUNNING && !list_first_data(proc_list_running, proc)) {
		assert(list_remove(proc_list_blocked, list_first_data(proc_list_blocked, proc)));
		assert(list_add(proc_list_running, proc));
	} else if (state == PROC_BLOCKED && !list_first_data(proc_list_blocked, proc)) {
		assert(list_remove(proc_list_running, list_first_data(proc_list_running, proc)));
		assert(list_add(proc_list_blocked, proc));
	}
	// else: Nothing to do!
}

void proc_exit(s32 status)
{
	struct proc *proc = proc_current();
	assert(proc && proc != idle_proc->data);

	struct node *running = list_first_data(proc_list_running, proc);
	if (!running || !list_remove(proc_list_running, running)) {
		struct node *blocked = list_first_data(proc_list_blocked, proc);
		assert(blocked && list_remove(proc_list_blocked, blocked));
	}

	// Force switch to other process
	memory_switch_dir(virtual_kernel_dir());
	current = NULL;

	printf("Process %s (%d) exited with status %d (%s)\n",
	       proc->name[0] ? proc->name : "UNKNOWN", proc->pid, status,
	       status == 0 ? "success" : "error");

	if (proc->memory->head) {
		printf("Process leaked memory:\n");
		u32 total = 0;
		struct node *iterator = proc->memory->head;
		while (iterator) {
			struct memory_proc_link *link = iterator->data;
			printf("\t-> 0x%x: %dB\n", link->vrange.base, link->vrange.size);
			total += link->vrange.size;
			iterator = iterator->next;
		}
		printf("\tTOTAL: %dB (%dKiB)\n", total, total >> 10);
	} else {
		printf("Process didn't leak memory!\n");
	}

	dev_remove_proc(proc);
	stack_destroy(proc->messages);
	list_destroy(proc->memory); // TODO: Decrement memory ref links
	virtual_destroy_dir(proc->page_dir);
	memset(proc, 0, sizeof(*proc));
	free(proc);

	proc_yield();
	assert_not_reached();
}

void proc_yield(void)
{
	__asm__ volatile("int $129");
}

void proc_stack_user_push(struct proc *proc, const void *data, u32 size)
{
	struct page_dir *prev;
	memory_backup_dir(&prev);
	memory_switch_dir(proc->page_dir);

	proc->stack.user_ptr -= size;
	memcpy_user((void *)proc->stack.user_ptr, data, size);

	memory_switch_dir(prev);
}

void proc_stack_kernel_push(struct proc *proc, const void *data, u32 size)
{
	struct page_dir *prev;
	memory_backup_dir(&prev);
	memory_switch_dir(proc->page_dir);

	proc->stack.kernel_ptr -= size;
	memcpy_user((void *)proc->stack.kernel_ptr, data, size);

	memory_switch_dir(prev);
}

struct proc *proc_make(enum proc_priv priv)
{
	struct proc *proc = zalloc(sizeof(*proc));
	fpu_init(proc);
	proc->pid = current_pid++;
	proc->priv = priv;
	proc->messages = stack_new();
	proc->memory = list_new();
	proc->state = PROC_RUNNING;
	proc->page_dir = virtual_create_dir();
	proc->quantum.val = PROC_QUANTUM;
	proc->quantum.cnt = 0;

	return proc;
}

void proc_make_regs(struct proc *proc)
{
	struct int_frame_user frame = { 0 };

	assert(proc->entry);
	frame.eip = proc->entry;

	// Allocate user stack with readonly lower and upper page boundary
	u32 user_stack = (u32)memory_alloc_with_boundary(proc->page_dir, PROC_STACK_SIZE,
							 MEMORY_CLEAR | MEMORY_USER);

	// Allocate kernel stack with readonly lower and upper page boundary
	u32 kernel_stack =
		(u32)memory_alloc_with_boundary(proc->page_dir, PROC_STACK_SIZE, MEMORY_CLEAR);

	proc->stack.user = user_stack + PROC_STACK_SIZE;
	proc->stack.user_ptr = proc->stack.user;

	proc->stack.kernel = kernel_stack + PROC_STACK_SIZE;
	proc->stack.kernel_ptr = proc->stack.kernel;

	frame.esp = proc->stack.kernel;
	frame.ebp = proc->stack.kernel;
	frame.useresp = proc->stack.user;

	// Init regs
	u8 is_kernel = proc->priv == PROC_PRIV_KERNEL;
	u32 data = is_kernel ? GDT_SUPER_DATA_OFFSET : GDT_USER_DATA_OFFSET;
	u32 code = is_kernel ? GDT_SUPER_CODE_OFFSET : GDT_USER_CODE_OFFSET;
	frame.gs = data;
	frame.fs = data;
	frame.es = data;
	frame.ds = data;
	frame.ss = data;
	frame.cs = code;
	frame.eflags = EFLAGS_ALWAYS | EFLAGS_INTERRUPTS;

	// Push frame as the values get popped (see int.asm)
	proc_stack_kernel_push(proc, &frame, sizeof(frame));

	// Push argc and argv // TODO
	u32 arg = 0;
	proc_stack_user_push(proc, &arg, sizeof(arg));
	proc_stack_user_push(proc, &arg, sizeof(arg));

	list_add(proc_list_running, proc);
}

// TODO: Procfs needs a simpler interface structure (memcmp and everything sucks)

static const char *procfs_parse_path(const char **path, u32 *pid)
{
	stac();
	while (**path == '/')
		(*path)++;

	while ((*path)[0] >= '0' && (*path)[0] <= '9') {
		*pid = *pid * 10 + ((*path)[0] - '0');
		(*path)++;
	}

	if (!*pid && !memcmp(*path, "self/", 5)) {
		*pid = proc_current()->pid;
		*path += 4;
	}
	clac();

	return *path;
}

struct procfs_message {
	u8 *data;
	u32 size;
};

static res procfs_read(const char *path, void *buf, u32 offset, u32 count, struct vfs_dev *dev)
{
	UNUSED(dev);
	u32 pid = 0;
	procfs_parse_path(&path, &pid);

	if (pid) {
		struct proc *p = proc_from_pid(pid);
		stac();
		if (!p || path[0] != '/') {
			clac();
			return -ENOENT;
		}
		clac();

		path++;
		if (!memcmp_user(path, "pid", 4)) {
			/* memcpy_user(buf, ((u8 *)p->pid) + offset, count); */
			stac();
			*(u32 *)buf = p->pid;
			clac();
			return count;
		} else if (!memcmp_user(path, "name", 5)) {
			memcpy_user(buf, p->name + offset, count);
			return count;
		} else if (!memcmp_user(path, "status", 7)) {
			const char *status = p->state == PROC_RUNNING ? "running" : "sleeping";
			memcpy_user(buf, status + offset, count);
			return count;
		}
	}

	return -ENOENT;
}

static res procfs_perm(const char *path, enum vfs_perm perm, struct vfs_dev *dev)
{
	UNUSED(path);
	UNUSED(dev);

	if (perm == VFS_EXEC)
		return -EACCES;
	else
		return EOK;
}

extern void proc_jump_userspace(void);

u32 _esp, _eip;
void proc_init(void)
{
	if (proc_list_running)
		panic("Already initialized processes!");

	proc_list_running = list_new();
	proc_list_blocked = list_new();
	proc_list_idle = list_new();

	// Procfs
	struct vfs *vfs = zalloc(sizeof(*vfs));
	vfs->type = VFS_PROCFS;
	vfs->read = procfs_read;
	vfs->perm = procfs_perm;
	vfs->data = NULL;
	struct vfs_dev *dev = zalloc(sizeof(*dev));
	strlcpy(dev->name, "proc", sizeof(dev->name));
	dev->type = DEV_CHAR;
	dev->vfs = vfs;
	vfs_add_dev(dev);
	assert(vfs_mount(dev, "/proc/") == EOK);

	// Idle proc
	// TODO: Reimplement hlt privileges in idle proc (SMEP!)
	struct proc *kernel_proc = proc_make(PROC_PRIV_NONE);
	assert(elf_load("idle", kernel_proc) == EOK);
	kernel_proc->state = PROC_BLOCKED;
	kernel_proc->quantum.val = 0;
	kernel_proc->quantum.cnt = 0;
	idle_proc = list_add(proc_list_idle, kernel_proc);
	list_remove(proc_list_running, list_first_data(proc_list_running, kernel_proc));

	// Init proc (root)
	struct proc *init = proc_make(PROC_PRIV_ROOT);
	assert(elf_load("init", init) == EOK);
	current = list_first_data(proc_list_running, init);

	_eip = init->entry;
	_esp = init->stack.user_ptr;

	// We'll shortly jump to usermode. Clear and protect every secret!
	memory_user_hook();

	tss_set_stack(init->stack.kernel);
	memory_switch_dir(init->page_dir);
	printf("Jumping to userspace!\n");

	// You're waiting for a train. A train that will take you far away...
	proc_jump_userspace();
	assert_not_reached();
}
