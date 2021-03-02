// MIT License, Copyright (c) 2020 Marvin Borner

#include <fs.h>
#include <load.h>
#include <mem.h>
#include <mm.h>
#include <str.h>

#define PROC_STACK_SIZE 0x4000

void proc_load(struct proc *proc, u32 entry)
{
	u32 stack;
	memory_alloc(proc->page_dir, PROC_STACK_SIZE, MEMORY_CLEAR, &stack);

	proc->regs.ebp = stack;
	proc->regs.useresp = stack;
	proc->regs.eip = entry;
	proc->entry = entry;
}

int bin_load(const char *path, struct proc *proc)
{
	struct stat s = { 0 };
	vfs_stat(path, &s);
	u32 data;
	memory_alloc(proc->page_dir, PAGE_ALIGN_UP(s.size), MEMORY_CLEAR, &data);
	if (!vfs_read(path, (void *)data, 0, s.size))
		return 1;

	strcpy(proc->name, path);
	proc_load(proc, data);

	return 0;
}
