// MIT License, Copyright (c) 2020 Marvin Borner

#include <fs.h>
#include <load.h>
#include <mem.h>
#include <mm.h>
#include <str.h>

#define PROC_STACK_SIZE 0x4000

/*void proc_load(struct proc *proc, u32 entry)
{
	u32 stack = (u32)memory_alloc(proc->page_dir, PROC_STACK_SIZE, MEMORY_USER | MEMORY_CLEAR);

	proc->regs.ebp = stack;
	proc->regs.useresp = stack;
	proc->regs.eip = entry;
	proc->entry = entry;
}*/

int bin_load(const char *path, struct proc *proc)
{
	struct stat s = { 0 };
	vfs_stat(path, &s);
	struct proc *current = proc_current();
	struct page_dir *prev = current ? current->page_dir : virtual_kernel_dir();

	u32 size = PAGE_ALIGN_UP(s.size);
	memory_switch_dir(proc->page_dir);
	u32 data = (u32)memory_alloc(proc->page_dir, size, MEMORY_USER | MEMORY_CLEAR);

	if (!vfs_read(path, (void *)data, 0, s.size)) {
		memory_switch_dir(prev);
		return 1;
	}

	strcpy(proc->name, path);

	u32 stack = (u32)memory_alloc(proc->page_dir, PROC_STACK_SIZE, MEMORY_USER | MEMORY_CLEAR);
	proc->regs.ebp = stack;
	proc->regs.useresp = stack;
	proc->regs.eip = data;
	proc->entry = data;

	memory_switch_dir(prev);
	return 0;
}
