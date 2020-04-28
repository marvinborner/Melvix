#include <stdint.h>
#include <stddef.h>
#include <kernel/tasks/process.h>
#include <kernel/tasks/userspace.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/system.h>
#include <kernel/lib/lib.h>
#include <kernel/memory/paging.h>
#include <kernel/memory/alloc.h>
#include <kernel/timer/timer.h>
#include <kernel/fs/elf.h>

uint32_t pid = 0;
struct process *root;
struct process *current_proc = NULL;

struct regs hold_root;

extern uint32_t stack_hold;

void scheduler(struct regs *regs)
{
	memcpy(&current_proc->registers, regs, sizeof(struct regs));

	timer_handler(regs);

	current_proc = current_proc->next;
	if (current_proc == NULL) {
		current_proc = root;
	}

	//debug("Task switch to %s", current_proc->name);

	while (current_proc->state == PROC_ASLEEP) {
		current_proc = current_proc->next;
		if (current_proc == NULL)
			current_proc = root;
	}

	memcpy(regs, &current_proc->registers, sizeof(struct regs));
	paging_switch_directory(current_proc->cr3);
}

void process_init(struct process *proc)
{
	root = proc;
	root->pid = pid++;
	root->next = NULL;
	root->thread = PROC_ROOT; // Unkillable
	root->state = PROC_RUNNING;

	current_proc = root;
	irq_install_handler(0, scheduler);
	userspace_enter(proc);
}

void process_kill(uint32_t pid)
{
	struct process *proc = process_from_pid(pid);

	if (proc == PID_NOT_FOUND)
		panic("Can't kill unknown PID");

	// flush(proc->stdout);
	// flush(proc->stderr);
	proc->state = PROC_ASLEEP;

	if (proc->parent != NULL) {
		//warn("Child had parent");
		//process_wake(proc->parent->pid);
	}
}

uint32_t process_spawn(struct process *process)
{
	process->next = root->next;
	root->next = process;
	process->state = PROC_RUNNING;

	process->parent = current_proc;

	//process_suspend(current_proc->pid);

	return process->pid;
}

int process_wait_gid(uint32_t gid, int *status)
{
	struct process *i = root;

	while (i != NULL) {
		if (i->gid == gid)
			if (i->state == PROC_ASLEEP) {
				*status = i->registers.ebx;
				return i->pid;
			}

		i = i->next;
	}

	return WAIT_OKAY;
}

int process_wait_pid(uint32_t pid, int *status)
{
	struct process *i = current_proc->next;

	while (i != NULL) {
		if (i->pid == pid) {
			if (i->state == PROC_ASLEEP) {
				*status = i->registers.ebx;
				return i->pid;
			} else {
				return WAIT_OKAY;
			}
		}
		i = i->next;
	}

	return WAIT_ERROR;
}

void process_suspend(uint32_t pid)
{
	struct process *proc = process_from_pid(pid);

	if (proc == PID_NOT_FOUND) {
		warn("couldn't find PID for suspension");
		return;
	}

	proc->state = PROC_ASLEEP;
}

void process_wake(uint32_t pid)
{
	struct process *proc = process_from_pid(pid);

	if (proc == PID_NOT_FOUND)
		return;

	proc->state = PROC_RUNNING;
}

uint32_t process_child(struct process *child, uint32_t pid)
{
	process_suspend(pid);

	struct process *parent = process_from_pid(pid);

	if (parent == PID_NOT_FOUND) {
		panic("Child process spawned without parent");
	}

	child->parent = parent;

	return process_spawn(child);
}

uint32_t process_fork(uint32_t pid)
{
	warn("Fork is not implemented");

	// With struct regs *regs
	/*struct page_directory *dir = paging_copy_user_directory(current_proc->cr3);
	struct process *proc = process_make_new();
	proc->cr3 = dir;
	memcpy(&proc->registers, regs, sizeof(struct regs));
	proc->registers.eax = proc->pid;
	proc->pid = current_proc->pid;

	process_spawn(proc);*/
	return pid++;
}

struct process *process_from_pid(uint32_t pid)
{
	struct process *proc = root;

	while (proc != NULL) {
		if (proc->pid == pid)
			return proc;

		proc = proc->next;
	}

	return PID_NOT_FOUND;
}

struct process *process_make_new()
{
	struct process *proc = (struct process *)kmalloc_a(sizeof(struct process));
	proc->registers.cs = 0x1B;
	proc->registers.ds = 0x23;
	proc->registers.ss = 0x23;
	proc->cr3 = paging_make_directory();

	proc->brk = 0x50000000;

	int i;
	for (i = 0; i < 1024; i++)
		proc->cr3->tables[i] = paging_root_directory->tables[i];

	proc->pid = pid++;

	return proc;
}

uint32_t kexec(char *path)
{
	struct process *proc = elf_load(path);
	if (proc == NULL)
		return -1;

	// TODO: Add stdin etc support (?)
	proc->stdin = NULL;
	proc->stdout = NULL;
	proc->stderr = NULL;
	process_init(proc);
}

uint32_t uexec(char *path)
{
	process_suspend(current_proc->pid);

	struct process *proc = elf_load(path);
	if (proc == NULL)
		return -1;

	proc->stdin = NULL;
	proc->stdout = NULL;
	proc->stderr = NULL;

	proc->parent = current_proc;
	proc->gid = current_proc->pid;
	process_spawn(proc);
	return 0;
}
