// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <fs.h>
#include <mem.h>
#include <print.h>
#include <proc.h>

void bin_load(char *path, struct proc *proc)
{
	char *data = read_file(path);
	u32 stack = (u32)malloc(0x1000) + 0x1000;

	proc->regs.ebp = stack;
	proc->regs.esp = stack;
	proc->regs.eip = (u32)data;
}
