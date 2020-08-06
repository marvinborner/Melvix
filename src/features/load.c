// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <fs.h>
#include <load.h>
#include <mem.h>
#include <print.h>
#include <proc.h>

void bin_load(char *path, struct proc *proc)
{
	char *data = read_file(path);
	u32 stack = (u32)malloc(0x1000) + 0x1000;

	proc->regs.ebp = (u32)stack;
	proc->regs.esp = (u32)stack;
	proc->regs.eip = (u32)data;

	proc->regs.ds = GDT_DATA_OFFSET;
	proc->regs.es = GDT_DATA_OFFSET;
	proc->regs.fs = GDT_DATA_OFFSET;
	proc->regs.gs = GDT_DATA_OFFSET;
	proc->regs.cs = GDT_CODE_OFFSET;
	proc->regs.eflags = EFLAGS_ALWAYS | EFLAGS_INTERRUPTS;
}
