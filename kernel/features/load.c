// MIT License, Copyright (c) 2020 Marvin Borner

#include <fs.h>
#include <load.h>
#include <mem.h>
#include <str.h>

void proc_load(struct proc *proc, void *data)
{
	u32 stack = (u32)malloc(0x2000) + 0x1000;

	proc->regs.ebp = (u32)stack;
	proc->regs.useresp = (u32)stack;
	proc->regs.eip = (u32)data;
	proc->entry = (u32)data;
}

int bin_load(const char *path, struct proc *proc)
{
	// TODO: Remove hardcoded filesize
	struct stat s = { 0 };
	vfs_stat(path, &s);
	char *data = malloc(s.size);
	if (!vfs_read(path, data, 0, s.size))
		return 1;

	strcpy(proc->name, path);
	proc_load(proc, data);

	return 0;
}
