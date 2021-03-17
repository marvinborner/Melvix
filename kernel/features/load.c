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
	UNUSED(path);
	UNUSED(proc);
	panic("Deprecated!\n");
#if 0
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
#endif
}

s32 elf_load(const char *path, struct proc *proc)
{
	if (!path || !memory_valid(path) || !proc)
		return -EFAULT;

	struct stat s = { 0 };
	s32 stat = vfs_stat(path, &s);
	if (stat != 0)
		return stat;

	struct elf_header header = { 0 };
	s32 read = vfs_read(path, &header, 0, sizeof(header));
	if (read < 0)
		return read;
	if (read != sizeof(header))
		return -ENOEXEC;

	strcpy(proc->name, path);

	// Valid?
	u8 *magic = header.ident;
	u8 valid_magic = magic[ELF_IDENT_MAG0] == ELF_MAG0 && magic[ELF_IDENT_MAG1] == ELF_MAG1 &&
			 magic[ELF_IDENT_MAG2] == ELF_MAG2 && magic[ELF_IDENT_MAG3] == ELF_MAG3 &&
			 magic[ELF_IDENT_CLASS] == ELF_IDENT_CLASS_32 &&
			 magic[ELF_IDENT_DATA] == ELF_IDENT_DATA_LSB;
	if (!valid_magic || (header.type != ELF_ETYPE_REL && header.type != ELF_ETYPE_EXEC) ||
	    header.version != 1 || header.machine != ELF_MACHINE_386)
		return -ENOEXEC;

	for (u32 i = 0; i < header.phnum; i++) {
		struct elf_program program = { 0 };
		if (vfs_read(path, &program, header.phoff + header.phentsize * i,
			     sizeof(program)) != sizeof(program))
			return -ENOEXEC;

		if (program.vaddr == 0)
			continue;

		if (program.vaddr <= 0x100000)
			return -ENOEXEC;

		struct page_dir *prev;
		memory_backup_dir(&prev);
		memory_switch_dir(proc->page_dir);

		struct memory_range vrange = memory_range_around(program.vaddr, program.memsz);
		struct memory_range prange = physical_alloc(vrange.size);
		virtual_map(proc->page_dir, prange, vrange.base, MEMORY_CLEAR | MEMORY_USER);

		if ((u32)vfs_read(path, (void *)program.vaddr, program.offset, program.filesz) !=
		    program.filesz) {
			print("OH NOSE!\n");
			memory_switch_dir(prev);
			return -ENOEXEC;
		}

		memory_switch_dir(prev);
	}

	struct page_dir *prev;
	memory_backup_dir(&prev);
	memory_switch_dir(proc->page_dir);

	u32 stack = (u32)memory_alloc(proc->page_dir, PROC_STACK_SIZE, MEMORY_USER | MEMORY_CLEAR);
	proc->regs.ebp = stack + PROC_STACK_SIZE - 1;
	proc->regs.useresp = stack + PROC_STACK_SIZE - 1;
	proc->regs.eip = header.entry;
	proc->entry = header.entry;

	memory_switch_dir(prev);
	return 0;
}
