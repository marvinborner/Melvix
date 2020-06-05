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

	/* serial_put('.'); */
	/* if (quantum == 0 || current_proc->state != PROC_RUNNING) { */
	/* 	quantum = 42; // For next process */
	/* } else { */
	/* 	quantum--; */
	/* 	locked = 0; */
	/* 	return; */
	/* } */

	serial_put('+');
	memcpy(&current_proc->regs, regs, sizeof(struct regs));

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

	memcpy(regs, &current_proc->regs, sizeof(struct regs));
	paging_switch_dir(current_proc->cr3);
	log("%x", regs->cs);
	if (regs->cs != 0x1B) {
		regs->gs = 0x23;
		regs->fs = 0x23;
		regs->es = 0x23;
		regs->ds = 0x23;
		regs->cs = 0x1B;
		regs->ss = 0x23;
	}

	locked = 0;
}

void process_force_switch()
{
	quantum = 0;
	timer_wait(1);
	//scheduler(regs);
}

u32 hl_eip;
u32 hl_esp;
void process_init(struct process *proc)
{
	log("Initializing process %d", pid);
	root = proc;
	root->pid = pid++;
	root->next = proc;
	root->state = PROC_RUNNING;

	current_proc = root;
	irq_install_handler(0, scheduler);

	hl_eip = proc->regs.eip;
	hl_esp = proc->regs.esp;
	paging_switch_dir(proc->cr3);

	debug("Jumping to userspace!");
	extern void userspace_jump();
	userspace_jump();
	panic("This should not happen!");
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

void process_suspend(u32 pid)
{
	debug("Suspending process %d", pid);
	process_print_tree();
	assert(pid != 1);

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

void no_entry()
{
	panic("No entry point given!");
}

struct process *process_make_new()
{
	debug("Making new process %d", pid);
	struct process *proc = (struct process *)valloc(sizeof(struct process));
	proc->regs.cs = 0x1B;
	proc->regs.ds = 0x23;
	proc->regs.ss = 0x23;
	proc->regs.eip = no_entry;
	proc->cr3 = paging_make_dir();
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