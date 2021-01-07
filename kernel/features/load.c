// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <cpu.h>
#include <def.h>
#include <fs.h>
#include <load.h>
#include <mem.h>
#include <proc.h>
#include <str.h>

int bin_load(char *path, struct proc *proc)
{
	// TODO: Remove hardcoded filesize
	char *data = malloc(0xffff);
	vfs_read(path, data, 0, 0xffff);

	u32 stack = (u32)malloc(0x2000) + 0x1000;

	proc->regs.ebp = (u32)stack;
	proc->regs.useresp = (u32)stack;
	proc->regs.eip = (u32)data;
	strcpy(proc->name, path + 1);

	return data ? 0 : 1;
}
