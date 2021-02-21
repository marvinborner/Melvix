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
struct proc *priority_proc = NULL;
struct list *proc_list = NULL;
struct node *idle_proc = NULL;
struct node *current = NULL;

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
		struct node *iterator = proc_list->head;
		while (iterator) {
			if (((struct proc *)iterator->data)->state == PROC_RUNNING) {
				current = iterator;
				break;
			}
			iterator = iterator->next;
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

	if (current == idle_proc)
		quantum = 0;

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

u8 proc_super(void)
{
	struct proc *proc = proc_current();
	if (proc)
		return proc->super;
	else if (current_pid == 0)
		return 1; // Kernel has super permissions
	else
		return 0; // Should never happen
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
		printf("Process %s exited with status %d (%s)\n", proc->name, status,
		       status == 0 ? "success" : "error");

	proc_clear_quantum(); // TODO: Add quantum to each process struct?
	sti();
	hlt();
}

void proc_yield(struct regs *r)
{
	proc_clear_quantum();
	scheduler(r);
}

void proc_enable_waiting(u32 id, enum proc_wait_type type)
{
	struct proc *proc_bak = proc_current();

	struct node *iterator = proc_list->head;
	while (iterator) {
		struct proc *p = iterator->data;
		struct proc_wait *w = &p->wait;

		if (!p || !w || w->id_cnt == 0) {
			iterator = iterator->next;
			continue;
		}

		current = list_first_data(proc_list, p);
		assert(w->id_cnt < PROC_MAX_WAIT_IDS);
		for (u32 i = 0; i < w->id_cnt; i++) {
			if (w->ids[i].magic == PROC_WAIT_MAGIC && w->ids[i].id == id &&
			    w->ids[i].type == type) {
				struct regs *r = &p->regs;
				if (w->ids[i].func)
					r->eax = (u32)w->ids[i].func((char *)r->ebx, (void *)r->ecx,
								     r->edx, r->esi);
				memset(&w->ids[i], 0, sizeof(w->ids[i]));
				p->wait.id_cnt--;
				p->state = PROC_RUNNING;
				break;
			}
		}

		iterator = iterator->next;
	}

	if (current->data != proc_bak)
		current = list_first_data(proc_list, proc_bak);
}

void proc_wait_for(u32 id, enum proc_wait_type type, s32 (*func)())
{
	u8 already_exists = 0;
	struct proc *p = proc_current();

	// Check if already exists
	for (u32 i = 0; i < p->wait.id_cnt; i++) {
		if (p->wait.ids[i].id == id && p->wait.ids[i].type == type) {
			assert(p->wait.ids[i].func == func);
			already_exists = 1;
		}
	}

	if (already_exists)
		goto end;

	assert(p->wait.id_cnt + 1 < PROC_MAX_WAIT_IDS);

	// Find slot
	struct proc_wait_identifier *slot = NULL;
	for (u32 i = 0; i < PROC_MAX_WAIT_IDS; i++) {
		if (p->wait.ids[i].magic != PROC_WAIT_MAGIC) {
			slot = &p->wait.ids[i];
			break;
		}
	}
	assert(slot != NULL);

	slot->magic = PROC_WAIT_MAGIC;
	slot->id = id;
	slot->type = type;
	slot->func = func;
	p->wait.id_cnt++;

end:
	p->state = PROC_SLEEPING;
}

struct proc *proc_make(void)
{
	struct proc *proc = zalloc(sizeof(*proc));
	proc->pid = current_pid++;
	proc->super = 0;
	proc->messages = stack_new();
	proc->state = PROC_RUNNING;

	if (current)
		list_add(proc_list, proc);

	return proc;
}

// TODO: Procfs needs a simpler interface structure (memcmp and everything sucks)

const char *procfs_parse_path(const char **path, u32 *pid)
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

enum stream_defaults procfs_stream(const char *path)
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

s32 procfs_write(const char *path, void *buf, u32 offset, u32 count, struct device *dev)
{
	u32 pid = 0;
	procfs_parse_path(&path, &pid);
	if (pid) {
		struct proc *p = proc_from_pid(pid);
		if (!p || path[0] != '/')
			return -1;

		path++;
		if (!memcmp(path, "msg", 4)) {
			// TODO: Messages should be copied by value not by reference
			stack_push_bot(p->messages, buf); // TODO: Use offset and count
			proc_enable_waiting(pid, PROC_WAIT_MSG);
			return count;
		} else if (!memcmp(path, "io/", 3)) {
			path += 3;
			enum stream_defaults id = procfs_stream(path);
			if (id == STREAM_UNKNOWN)
				return -1;

			// Put proc log/err messages to serial console for debugging
			if (id == STREAM_LOG || id == STREAM_ERR)
				print_app(id, p->name, (char *)buf);

			struct stream *stream = &p->streams[id];
			assert(stream->offset_write + count < STREAM_MAX_SIZE); // TODO: Resize
			memcpy((char *)(stream->data + stream->offset_write), buf, count);
			stream->offset_write += count;
			proc_enable_waiting(dev->id, PROC_WAIT_DEV);
			return count;
		}
	}

	printf("%s - off: %d, cnt: %d, buf: %x, dev %x\n", path, offset, count, buf, dev);
	return -1;
}

s32 procfs_read(const char *path, void *buf, u32 offset, u32 count, struct device *dev)
{
	(void)dev;
	u32 pid = 0;
	procfs_parse_path(&path, &pid);

	if (pid) {
		struct proc *p = proc_from_pid(pid);
		if (!p || path[0] != '/')
			return -1;

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
				return -1; // This shouldn't happen
			} else {
				u8 *msg = stack_pop(p->messages);
				if (!msg)
					return -1;
				memcpy(buf, msg + offset, count);
				return count;
			}
		} else if (!memcmp(path, "io/", 3)) {
			path += 3;
			enum stream_defaults id = procfs_stream(path);
			if (id == STREAM_UNKNOWN)
				return -1;
			struct stream *stream = &p->streams[id];
			memcpy(buf, stream->data + stream->offset_read, count);
			stream->offset_read += count;
			return count;
		}
	}

	return -1;
}

s32 procfs_wait(const char *path, s32 (*func)(), struct device *dev)
{
	u32 pid = 0;
	procfs_parse_path(&path, &pid);

	if (pid) {
		struct proc *p = proc_from_pid(pid);
		if (!p || path[0] != '/')
			return -1;

		path++;
		if (!memcmp(path, "msg", 4)) {
			proc_wait_for(pid, PROC_WAIT_MSG, func);
			return 1;
		} else {
			proc_wait_for(dev->id, PROC_WAIT_DEV, func);
			return 1;
		}
	}

	return -1;
}

u8 procfs_perm(const char *path, enum vfs_perm perm, struct device *dev)
{
	(void)path;
	(void)dev;

	if (perm == VFS_EXEC)
		return 0;
	else
		return 1;
}

u8 procfs_ready(const char *path, struct device *dev)
{
	(void)dev;

	u32 pid = 0;
	procfs_parse_path(&path, &pid);

	if (pid) {
		struct proc *p = proc_from_pid(pid);
		if (!p || path[0] != '/')
			return -1;

		path++;
		if (!memcmp(path, "msg", 4)) {
			return stack_empty(p->messages) == 0;
		} else if (!memcmp(path, "io/", 3)) {
			path += 3;
			enum stream_defaults id = procfs_stream(path);
			if (id == STREAM_UNKNOWN)
				return -1;
			struct stream *stream = &p->streams[id];
			return stream->data[stream->offset_read] != 0;
		}
	}

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
	struct vfs *vfs = zalloc(sizeof(*vfs));
	vfs->type = VFS_PROCFS;
	vfs->read = procfs_read;
	vfs->write = procfs_write;
	vfs->wait = procfs_wait;
	vfs->perm = procfs_perm;
	vfs->ready = procfs_ready;
	vfs->data = NULL;
	struct device *dev = zalloc(sizeof(*dev));
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
	((struct proc *)new->data)->super = 1;

	u32 argc = 2;
	char **argv = malloc(sizeof(*argv) * (argc + 1));
	argv[0] = strdup("init");
	argv[1] = (char *)boot_passed->vbe;
	argv[2] = NULL;

	((u32 *)_esp)[0] = argc; // First argument (argc)
	((u32 *)_esp)[1] = (u32)argv; // Second argument (argv)

	printf("Jumping to userspace!\n");
	proc_jump_userspace();
	while (1) {
	};
}
