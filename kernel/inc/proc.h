// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef PROC_H
#define PROC_H

#include <def.h>
#include <event.h>
#include <interrupts.h>
#include <list.h>
#include <sys.h>

#define PROC_QUANTUM 42 // Milliseconds

#define EFLAGS_ALWAYS 0x2 // Always one
#define EFLAGS_INTERRUPTS 0x200 // Enable interrupts

#define GDT_USER_CODE_OFFSET 0x1b // User code segment offset in GDT (with ring3 mask)
#define GDT_USER_DATA_OFFSET 0x23 // User data segment offset in GDT (with ring3 mask)

enum proc_state { PROC_DEFAULT, PROC_IN_EVENT, PROC_RESOLVED };

struct proc {
	u32 pid;
	char name[32];
	struct regs regs;
	struct regs regs_backup;
	enum proc_state state;
	struct list *messages;
};

struct proc_message {
	struct proc *src;
	struct proc *dest;
	struct message *msg;
};

struct proc *kernel_proc;

void proc_init();
void proc_print();
struct proc *proc_current();
void proc_send(struct proc *src, struct proc *dest, enum message_type type, void *data);
struct proc_message *proc_receive(struct proc *proc);
struct proc *proc_from_pid(u32 pid);
void proc_exit(struct proc *proc, int status);
void proc_yield();
struct proc *proc_make();

#endif
