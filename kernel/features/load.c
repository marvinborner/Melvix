// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <fs.h>
#include <mem.h>
#include <proc.h>
#include <str.h>

void bin_load(char *path, struct proc *proc)
{
	char *data = read_file(path);
	u32 stack = (u32)malloc(0x1000) + 0x1000;

	proc->regs.ebp = (u32)stack;
	proc->regs.esp = (u32)stack;
	proc->regs.useresp = (u32)stack;
	proc->regs.eip = (u32)data;
	strcpy(proc->name, path + 1);
}
