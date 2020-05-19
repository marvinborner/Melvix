#include <fs/elf.h>
#include <interrupts/interrupts.h>
#include <io/io.h>
#include <lib/lib.h>
#include <memory/alloc.h>
#include <memory/paging.h>
#include <stddef.h>
#include <stdint.h>
#include <system.h>
#include <tasks/process.h>
#include <tasks/userspace.h>
#include <timer/timer.h>

u32 pid = 0;
u32 quantum = 42; // In ms
struct process *root;
struct process *current_proc = NULL;

struct regs hold_root;

extern u32 stack_hold;

void scheduler(struct regs *regs)
{
	static int locked = 0;
	spinlock(&locked);

	serial_put('.');
	if (quantum == 0 || current_proc->state != PROC_RUNNING) {
		quantum = 42; // For next process
	} else {
		quantum--;
		locked = 0;
		sti();
		return;
	}

	serial_put('+');
	memcpy(&current_proc->registers, regs, sizeof(struct regs));

	timer_handler(regs);

	current_proc = current_proc->next;
	if (current_proc == NULL)
		current_proc = root;

	/* debug("Max pid: %d", pid); */
	/* debug("Task switch to %s with pid %d", current_proc->name, current_proc->pid); */

	while (current_proc->state == PROC_ASLEEP) {
		current_proc = current_proc->next;
		if (current_proc == NULL) {
			warn("No next process!");
			current_proc = root;
		}
	}

	memcpy(regs, &current_proc->registers, sizeof(struct regs));
	paging_switch_directory(current_proc->cr3);
	locked = 0;
	cli();
}

void process_force_switch()
{
	quantum = 0;
	timer_wait(1);
	//scheduler(regs);
}

void process_init(struct process *proc)
{
	log("Initializing process %d", pid);
	root = proc;
	root->pid = pid++;
	root->next = proc;
	root->state = PROC_RUNNING;

	current_proc = root;
	irq_install_handler(0, scheduler);
	userspace_enter(proc);
}

// Only for debugging purposes
void process_print_tree()
{
	serial_put('\n');
	info("PROCESS TREE");
	struct process *proc = root;

	while (proc != NULL) {
		info("%s with PID %d (state: %s)", proc->name, proc->pid,
		     proc->state == PROC_RUNNING ? "running" : "sleeping");
		proc = proc->next;
		if (proc->pid == 1)
			break;
	}
	serial_put('\n');
}

u32 process_spawn(struct process *process)
{
	debug("Spawning process %d", process->pid);
	process->next = current_proc->next;
	current_proc->next = process;
	process->state = PROC_RUNNING;

	process->parent = current_proc;
	process_print_tree();

	//process_suspend(current_proc->pid);

	return process->pid;
}

u32 process_wait_pid(u32 pid, u32 *status)
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

void process_suspend(u32 pid)
{
	debug("Suspending process %d", pid);
	process_print_tree();
	if (pid == 1)
		panic("Root process died");

	struct process *proc = process_from_pid(pid);

	if (proc == PID_NOT_FOUND) {
		warn("Couldn't find PID for suspension");
		return;
	}

	proc->state = PROC_ASLEEP;
	process_force_switch();
}

void process_wake(u32 pid)
{
	debug("Waking process %d", pid);
	struct process *proc = process_from_pid(pid);

	if (proc == PID_NOT_FOUND)
		return;

	proc->state = PROC_RUNNING;
	process_force_switch();
}

struct process *process_from_pid(u32 pid)
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
	debug("Making new process %d", pid);
	struct process *proc = (struct process *)valloc(sizeof(struct process));
	proc->registers.cs = 0x1B;
	proc->registers.ds = 0x23;
	proc->registers.ss = 0x23;
	proc->cr3 = paging_make_directory(1);

	proc->brk = 0x50000000;

	for (int i = 0; i < 1024; i++)
		proc->cr3[i] = kernel_page_directory[i];

	proc->pid = pid++;
	return proc;
}

u32 kexec(char *path)
{
	debug("Starting kernel process %s", path);
	struct process *proc = elf_load(path);
	if (proc == NULL)
		return -1;

	process_init(proc);
	process_force_switch();
	return 0;
}

u32 uexec(char *path)
{
	debug("Starting user process %s", path);

	struct process *proc = elf_load(path);
	if (proc == NULL)
		return -1;

	process_spawn(proc);
	process_suspend(current_proc->pid);
	log("Spawned");
	return 0;
}

u32 uspawn(char *path)
{
	debug("Spawning user process %s", path);

	struct process *proc = elf_load(path);
	if (proc == NULL)
		return -1;

	process_spawn(proc);
	log("Spawned");
	process_force_switch();
	return 0;
}