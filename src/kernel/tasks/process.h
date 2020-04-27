#ifndef MELVIX_PROCESS_H
#define MELVIX_PROCESS_H

#include <stdint.h>
#include <kernel/memory/paging.h>
#include <kernel/interrupts/interrupts.h>

struct mmap {
	uint32_t text;
	uint32_t bss;
	uint32_t data;
	uint32_t stack;
};

struct process {
	struct page_directory *cr3;
	struct regs registers;

	uint32_t pid;
	uint32_t gid;
	char *name;

	int state;
	int thread;

	uint32_t stdin;
	uint32_t stdout;
	uint32_t stderr;

	uint32_t brk;
	uint32_t handlers[6];

	struct process *parent;
	struct process *next;
};

void process_kill(uint32_t pid);

uint32_t process_spawn(struct process *process);

void process_suspend(uint32_t pid);
void process_wake(uint32_t pid);
uint32_t process_child(struct process *process, uint32_t pid);
uint32_t process_fork(uint32_t pid);

int process_wait_gid(uint32_t gid, int *status);
int process_wait_pid(uint32_t pid, int *status);

struct process *process_from_pid(uint32_t pid);

void process_init(struct process *proc);

struct process *process_make_new();

extern struct process *current_proc;

extern uint32_t stack_hold;

#define PID_NOT_FOUND ((struct process *)0xFFFFFFFF)

#define PROC_RUNNING 0
#define PROC_ASLEEP 1

#define PROC_THREAD 0
#define PROC_PROC 1
#define PROC_ROOT 2

#define WAIT_ERROR (-1)
#define WAIT_OKAY 0

#endif