// MIT License, Copyright (c) 2020 Marvin Borner

#include <errno.h>
#include <fs.h>
#include <load.h>
#include <mem.h>
#include <mm.h>
#include <str.h>

#include <print.h>

#define PROC_STACK_SIZE 0x4000

s32 bin_load(const char *path, struct proc *proc)
{
	if (!path || !memory_valid(path) || !proc)
		return -EFAULT;

	struct stat s = { 0 };
	memory_bypass_enable();
	s32 stat = vfs_stat(path, &s);
	memory_bypass_disable();
	if (stat != 0)
		return stat;

	strcpy(proc->name, path);

	struct page_dir *prev;
	memory_backup_dir(&prev);
	memory_switch_dir(proc->page_dir);

	u32 size = PAGE_ALIGN_UP(s.size);
	u32 data = (u32)memory_alloc(proc->page_dir, size, MEMORY_USER | MEMORY_CLEAR);

	memory_bypass_enable();
	s32 read = vfs_read(proc->name, (void *)data, 0, s.size);
	memory_bypass_disable();
	if (read <= 0) {
		memory_switch_dir(prev);
		return read;
	}

	u32 stack = (u32)memory_alloc(proc->page_dir, PROC_STACK_SIZE, MEMORY_USER | MEMORY_CLEAR);
	proc->regs.ebp = stack;
	proc->regs.useresp = stack;
	proc->regs.eip = data;
	proc->entry = data;

	memory_switch_dir(prev);
	return 0;
}
