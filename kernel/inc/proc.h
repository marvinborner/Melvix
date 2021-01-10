// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef PROC_H
#define PROC_H

#include <def.h>
#include <interrupts.h>
#include <list.h>
#include <sys.h>

#define PROC_QUANTUM 42 // Milliseconds or something // TODO

#define EFLAGS_ALWAYS 0x2 // Always one
#define EFLAGS_INTERRUPTS 0x200 // Enable interrupts

#define GDT_USER_CODE_OFFSET 0x1b // User code segment offset in GDT (with ring3 mask)
#define GDT_USER_DATA_OFFSET 0x23 // User data segment offset in GDT (with ring3 mask)

enum proc_state { PROC_RUNNING, PROC_SLEEPING };

struct proc {
	u32 pid;
	char name[32];
	struct regs regs;
	struct regs regs_backup;
	enum proc_state state;
	struct list *messages;
};

struct proc *kernel_proc;

void scheduler(struct regs *regs);
void proc_init(void);
void proc_print(void);
struct proc *proc_current(void);
struct proc *proc_from_pid(u32 pid);
void proc_exit(struct proc *proc, int status);
void proc_yield(struct regs *r);
struct proc *proc_make(void);

#endif
