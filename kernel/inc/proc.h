// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef PROC_H
#define PROC_H

#include <def.h>
#include <event.h>
#include <interrupts.h>
#include <list.h>

#define PROC_QUANTUM 42 // Milliseconds

#define EFLAGS_ALWAYS 0x2 // Always one
#define EFLAGS_INTERRUPTS 0x200 // Enable interrupts

#define GDT_USER_CODE_OFFSET 0x1b // User code segment offset in GDT (with ring3 mask)
#define GDT_USER_DATA_OFFSET 0x23 // User data segment offset in GDT (with ring3 mask)

struct proc {
	u32 pid;
	char name[32];
	struct regs regs;
	struct list *events;
};

struct proc_event {
	struct event_descriptor *desc;
	void *data;
};

void proc_init();
void proc_print();
struct proc *proc_current();
void proc_exit(struct proc *proc, int status);
struct proc *proc_make();

#endif
