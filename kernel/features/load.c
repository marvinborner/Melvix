// MIT License, Copyright (c) 2020 Marvin Borner

#include <fs.h>
#include <load.h>
#include <mem.h>
#include <mm.h>
#include <str.h>

// TODO: Fix pdi < 256!
#define PROC_DATA_ADDR 0xc000000

#define PROC_STACK_SIZE 0x4000
#define PROC_STACK_ADDR (PROC_DATA_ADDR - 256)

void proc_load(struct proc *proc, u32 entry)
{
	/* memory_dir_switch(proc->page_dir); */
	u32 paddr = physical_alloc(PROC_STACK_SIZE);
	virtual_map(proc->page_dir, PROC_STACK_ADDR, paddr, PROC_STACK_SIZE,
		    MEMORY_USER | MEMORY_CLEAR);

	proc->regs.ebp = PROC_STACK_ADDR;
	proc->regs.useresp = PROC_STACK_ADDR;
	proc->regs.eip = entry;
	proc->entry = entry;
}

int bin_load(const char *path, struct proc *proc)
{
	struct stat s = { 0 };
	vfs_stat(path, &s);
	struct proc *current = proc_current();
	struct page_dir *prev = current ? current->page_dir : memory_kernel_dir();

	u32 size = PAGE_ALIGN_UP(s.size);
	memory_dir_switch(proc->page_dir);
	u32 paddr = physical_alloc(size);
	virtual_map(proc->page_dir, PROC_DATA_ADDR, paddr, size, MEMORY_USER | MEMORY_CLEAR);

	if (!vfs_read(path, (void *)PROC_DATA_ADDR, 0, s.size)) {
		memory_dir_switch(prev);
		return 1;
	}

	strcpy(proc->name, path);
	proc_load(proc, PROC_DATA_ADDR);

	memory_dir_switch(prev);
	return 0;
}
