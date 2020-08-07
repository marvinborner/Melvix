// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef PROC_H
#define PROC_H

#include <def.h>
#include <interrupts.h>

#define EFLAGS_ALWAYS 0x2 // Always one
#define EFLAGS_INTERRUPTS 0x200 // Enable interrupts

#define GDT_USER_CODE_OFFSET 0x1b // User code segment offset in GDT (with ring3 mask)
#define GDT_USER_DATA_OFFSET 0x23 // User data segment offset in GDT (with ring3 mask)

enum state { PROC_RUNNING, PROC_ASLEEP };

struct proc {
	u32 pid;
	enum state state;
	char name[32];
	struct regs regs;
	/* struct proc *parent; */
	struct proc *next;
};

void proc_init();
void proc_print();
struct proc *proc_make();

#endif
