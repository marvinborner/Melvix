// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef PROC_H
#define PROC_H

#include <def.h>
#include <interrupts.h>

#define EFLAGS_ALWAYS 0x2 // Always one
#define EFLAGS_INTERRUPTS 0x200 // Enable interrupts

#define GDT_DATA_OFFSET 0x10 // Data segment offset in GDT
#define GDT_CODE_OFFSET 0x8 // Code segment offset in GDT

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
