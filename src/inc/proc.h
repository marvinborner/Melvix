// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef PROC_H
#define PROC_H

#include <def.h>
#include <interrupts.h>

enum state { PROC_RUNNING, PROC_ASLEEP };

struct proc {
	u32 pid;
	enum state state;
	struct regs regs;
	/* struct proc *parent; */
	struct proc *next;
};

void proc_init();
struct proc *proc_make();

#endif
