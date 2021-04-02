// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <boot.h>
#include <cpu.h>
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

// TODO: Use less memcpy and only copy relevant registers
// TODO: 20 priority queues (https://www.kernel.org/doc/html/latest/scheduler/sched-nice-design.html)
HOT FLATTEN void scheduler(struct regs *regs)
{
	spinlock(&locked);

	if (RING(regs) == 3)
		PROC(current)->ticks.user++;
	else
		PROC(current)->ticks.kernel++;

	if (PROC(current)->quantum.cnt >= PROC(current)->quantum.val) {
		PROC(current)->quantum.cnt = 0;
	} else {
		PROC(current)->quantum.cnt++;
		locked = 0;
		return;
	}

	memcpy(&PROC(current)->regs, regs, sizeof(*regs));

	if (current->next) {
		current = current->next;
	} else if (proc_list_running->head) {
		current = proc_list_running->head;
	} else {
		current = idle_proc;
	}

	memory_switch_dir(PROC(current)->page_dir);
	memcpy(regs, &PROC(current)->regs, sizeof(*regs));

	locked = 0;
}

void proc_print(void)
{
	struct node *node = proc_list_running->head;
	struct proc *proc = NULL;

	printf("--- PROCESSES ---\n");
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
	if (state == PROC_RUNNING && !list_first_data(proc_list_running, proc)) {
		assert(list_remove(proc_list_blocked, list_first_data(proc_list_blocked, proc)));
		assert(list_add(proc_list_running, proc));
	} else if (state == PROC_BLOCKED && !list_first_data(proc_list_blocked, proc)) {
		assert(list_remove(proc_list_running, list_first_data(proc_list_running, proc)));
		assert(list_add(proc_list_blocked, proc));
	}
	// else: Nothing to do!
}

void proc_exit(struct proc *proc, struct regs *r, s32 status)
{
	assert(proc != idle_proc->data);

	struct node *running = list_first_data(proc_list_running, proc);
	if (!running || !list_remove(proc_list_running, running)) {
		struct node *blocked = list_first_data(proc_list_blocked, proc);
		assert(blocked && list_remove(proc_list_blocked, blocked));
		// Idle procs can't be killed -> assertion failure.
	}

	if (current->data == proc) {
		current = idle_proc;
		memcpy(r, &PROC(idle_proc)->regs, sizeof(*r));
	}

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

	stack_destroy(proc->messages);
	list_destroy(proc->memory); // TODO: Decrement memory ref links
	virtual_destroy_dir(proc->page_dir);

	free(proc);

	proc_yield(r);
}

void proc_yield(struct regs *r)
{
	proc_reset_quantum(PROC(current));
	scheduler(r);
}

void proc_block(u32 id, enum proc_block_type type, u32 func_ptr)
{
	u8 already_exists = 0;
	struct proc *p = proc_current();

	// Check if already exists
	for (u32 i = 0; i < p->block.id_cnt; i++) {
		if (p->block.ids[i].id == id && p->block.ids[i].type == type) {
			assert(p->block.ids[i].func_ptr == func_ptr);
			already_exists = 1;
		}
	}

	if (already_exists)
		goto end;

	assert(p->block.id_cnt + 1 < PROC_MAX_BLOCK_IDS);

	// Find slot
	struct proc_block_identifier *slot = NULL;
	for (u32 i = 0; i < PROC_MAX_BLOCK_IDS; i++) {
		if (p->block.ids[i].magic != PROC_BLOCK_MAGIC) {
			slot = &p->block.ids[i];
			break;
		}
	}
	assert(slot);

	slot->magic = PROC_BLOCK_MAGIC;
	slot->id = id;
	slot->type = type;
	slot->func_ptr = func_ptr;
	p->block.id_cnt++;

end:
	proc_state(p, PROC_BLOCKED);
}

void proc_unblock(u32 id, enum proc_block_type type)
{
	struct page_dir *dir_bak;
	memory_backup_dir(&dir_bak);

	struct node *proc_bak = current;
	if (!proc_bak)
		return;

	struct node *iterator = proc_list_blocked->head;
	while (iterator) {
		struct proc *p = iterator->data;
		struct proc_block *w = &p->block;

		if (!p || !w || w->id_cnt == 0) {
			iterator = iterator->next;
			continue;
		}

		current = list_first_data(proc_list_blocked, p);
		assert(w->id_cnt < PROC_MAX_BLOCK_IDS);
		for (u32 i = 0; i < w->id_cnt; i++) {
			if (w->ids[i].magic == PROC_BLOCK_MAGIC && w->ids[i].id == id &&
			    w->ids[i].type == type) {
				struct regs *r = &p->regs;
				u32 (*func)(u32, u32, u32, u32) =
					(u32(*)(u32, u32, u32, u32))w->ids[i].func_ptr;
				if (w->ids[i].func_ptr) {
					memory_switch_dir(p->page_dir);
					r->eax = func(r->ebx, r->ecx, r->edx, r->esi);
					memory_switch_dir(dir_bak);
				}
				memset(&w->ids[i], 0, sizeof(w->ids[i]));
				p->block.id_cnt--;
				proc_state(p, PROC_RUNNING);
				break;
			}
		}

		iterator = iterator->next;
	}

	if (current != proc_bak)
		current = proc_bak;
}

struct proc *proc_make(enum proc_priv priv)
{
	struct proc *proc = zalloc(sizeof(*proc));
	proc->pid = current_pid++;
	proc->priv = priv;
	proc->messages = stack_new();
	proc->memory = list_new();
	proc->state = PROC_RUNNING;
	proc->page_dir = virtual_create_dir();
	proc->quantum.val = PROC_QUANTUM;
	proc->quantum.cnt = 0;

	// Init regs
	u8 is_kernel = priv == PROC_PRIV_KERNEL;
	u32 data = is_kernel ? GDT_SUPER_DATA_OFFSET : GDT_USER_DATA_OFFSET;
	u32 code = is_kernel ? GDT_SUPER_CODE_OFFSET : GDT_USER_CODE_OFFSET;
	proc->regs.gs = data;
	proc->regs.fs = data;
	proc->regs.es = data;
	proc->regs.ds = data;
	proc->regs.ss = data;
	proc->regs.cs = code;
	proc->regs.eflags = EFLAGS_ALWAYS | EFLAGS_INTERRUPTS;

	list_add(proc_list_running, proc);

	return proc;
}

void proc_stack_push(struct proc *proc, u32 data)
{
	assert(proc->regs.useresp > sizeof(data));

	struct page_dir *prev;
	memory_backup_dir(&prev);
	memory_switch_dir(proc->page_dir);

	proc->regs.useresp -= sizeof(data);
	*(u32 *)proc->regs.useresp = data;

	memory_switch_dir(prev);
}

// TODO: Procfs needs a simpler interface structure (memcmp and everything sucks)

static const char *procfs_parse_path(const char **path, u32 *pid)
{
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

	return *path;
}

static enum stream_defaults procfs_stream(const char *path)
{
	if (!memcmp(path, "in", 3)) {
		return STREAM_IN;
	} else if (!memcmp(path, "out", 4)) {
		return STREAM_OUT;
	} else if (!memcmp(path, "err", 4)) {
		return STREAM_ERR;
	} else if (!memcmp(path, "log", 4)) {
		return STREAM_LOG;
	} else {
		return STREAM_UNKNOWN;
	}
}

struct procfs_message {
	u8 *data;
	u32 size;
};

static res procfs_write(const char *path, void *buf, u32 offset, u32 count, struct device *dev)
{
	u32 pid = 0;
	procfs_parse_path(&path, &pid);
	if (pid) {
		struct proc *p = proc_from_pid(pid);
		if (!p || path[0] != '/')
			return -ENOENT;

		path++;
		if (!memcmp(path, "msg", 4)) {
			void *msg_data = malloc(count);
			memcpy(msg_data, buf, count);
			struct procfs_message *msg = malloc(sizeof(*msg));
			msg->data = msg_data;
			msg->size = count;
			stack_push_bot(p->messages, msg); // TODO: Use offset
			proc_unblock(pid, PROC_BLOCK_MSG);
			return count;
		} else if (!memcmp(path, "io/", 3)) {
			path += 3;
			enum stream_defaults id = procfs_stream(path);
			if (id == STREAM_UNKNOWN)
				return -ENOENT;

			// Put proc log/err messages to serial console for debugging
			if (id == STREAM_LOG || id == STREAM_ERR)
				print_app(id, p->name, (char *)buf);

			struct stream *stream = &p->streams[id];
			assert(stream->offset_write + count < STREAM_MAX_SIZE); // TODO: Resize
			memcpy((char *)(stream->data + stream->offset_write), buf, count);
			stream->offset_write += count;
			proc_unblock(dev->id, PROC_BLOCK_DEV);
			return count;
		}
	}

	printf("ERR: %s - off: %d, cnt: %d, buf: %x, dev %x\n", path, offset, count, buf, dev);
	return -ENOENT;
}

static res procfs_read(const char *path, void *buf, u32 offset, u32 count, struct device *dev)
{
	(void)dev;
	u32 pid = 0;
	procfs_parse_path(&path, &pid);

	if (pid) {
		struct proc *p = proc_from_pid(pid);
		if (!p || path[0] != '/')
			return -ENOENT;

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
				return -EIO; // This shouldn't happen
			} else {
				struct procfs_message *msg = stack_pop(p->messages);
				if (!msg)
					return -EIO;
				memcpy(buf, msg->data + offset, MIN(count, msg->size));
				free(msg->data);
				free(msg);
				return MIN(count, msg->size);
			}
		} else if (!memcmp(path, "io/", 3)) {
			path += 3;
			enum stream_defaults id = procfs_stream(path);
			if (id == STREAM_UNKNOWN)
				return -ENOENT;
			struct stream *stream = &p->streams[id];
			memcpy(buf, stream->data + stream->offset_read, count);
			stream->offset_read += count;
			return count;
		}
	}

	return -ENOENT;
}

static res procfs_block(const char *path, u32 func_ptr, struct device *dev)
{
	u32 pid = 0;
	procfs_parse_path(&path, &pid);

	if (pid) {
		struct proc *p = proc_from_pid(pid);
		if (!p || path[0] != '/')
			return -ENOENT;

		path++;
		if (!memcmp(path, "msg", 4)) {
			proc_block(pid, PROC_BLOCK_MSG, func_ptr);
			return EOK;
		} else {
			proc_block(dev->id, PROC_BLOCK_DEV, func_ptr);
			return EOK;
		}
	}

	return -ENOENT;
}

static res procfs_perm(const char *path, enum vfs_perm perm, struct device *dev)
{
	(void)path;
	(void)dev;

	if (perm == VFS_EXEC)
		return -EACCES;
	else
		return EOK;
}

static res procfs_ready(const char *path, struct device *dev)
{
	(void)dev;

	u32 pid = 0;
	procfs_parse_path(&path, &pid);

	if (pid) {
		struct proc *p = proc_from_pid(pid);
		if (!p || path[0] != '/')
			return -ENOENT;

		path++;
		if (!memcmp(path, "msg", 4)) {
			return stack_empty(p->messages) == 0;
		} else if (!memcmp(path, "io/", 3)) {
			path += 3;
			enum stream_defaults id = procfs_stream(path);
			if (id == STREAM_UNKNOWN)
				return -ENOENT;
			struct stream *stream = &p->streams[id];
			return stream->data[stream->offset_read] != 0;
		}
	}

	return 1;
}

extern void proc_jump_userspace(void);

u32 _esp, _eip;
NORETURN void proc_init(void)
{
	if (proc_list_running)
		panic("Already initialized processes!");

	cli();
	scheduler_enable();
	proc_list_running = list_new();
	proc_list_blocked = list_new();
	proc_list_idle = list_new();

	// Procfs
	struct vfs *vfs = zalloc(sizeof(*vfs));
	vfs->type = VFS_PROCFS;
	vfs->read = procfs_read;
	vfs->write = procfs_write;
	vfs->block = procfs_block;
	vfs->perm = procfs_perm;
	vfs->ready = procfs_ready;
	vfs->data = NULL;
	struct device *dev = zalloc(sizeof(*dev));
	dev->name = "proc";
	dev->type = DEV_CHAR;
	dev->vfs = vfs;
	device_add(dev);
	assert(vfs_mount(dev, "/proc/") == EOK);

	// Idle proc
	struct proc *kernel_proc = proc_make(PROC_PRIV_KERNEL);
	assert(elf_load("/bin/idle", kernel_proc) == EOK);
	kernel_proc->state = PROC_BLOCKED;
	kernel_proc->quantum.val = 0;
	kernel_proc->quantum.cnt = 0;
	idle_proc = list_add(proc_list_idle, kernel_proc);
	list_remove(proc_list_running, list_first_data(proc_list_running, kernel_proc));

	// Init proc (root)
	struct proc *init = proc_make(PROC_PRIV_ROOT);
	assert(elf_load("/bin/init", init) == EOK);
	proc_stack_push(init, 0);
	current = list_first_data(proc_list_running, init);

	_eip = init->regs.eip;
	_esp = init->regs.useresp;

	// We'll shortly jump to usermode. Clear and protect every secret!
	memory_user_hook();

	memory_switch_dir(init->page_dir);
	printf("Jumping to userspace!\n");

	// You're waiting for a train. A train that will take you far away...
	proc_jump_userspace();

	panic("Returned from limbo!\n");
}
