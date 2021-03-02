// MIT License, Copyright (c) 2020 Marvin Borner

#include <fs.h>
#include <load.h>
#include <mem.h>
#include <mm.h>
#include <str.h>

#define PROC_STACK_SIZE 0x4000

void proc_load(struct proc *proc, void *data)
{
	u32 stack;
	memory_alloc(proc->page_dir, PROC_STACK_SIZE, MEMORY_CLEAR, &stack);
	u32 ptr = stack + PROC_STACK_SIZE - 1;

	proc->regs.ebp = (u32)ptr;
	proc->regs.useresp = (u32)ptr;
	proc->regs.eip = (u32)data;
	proc->entry = (u32)data;
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
	proc_load(proc, (void *)data);

	return 0;
}
