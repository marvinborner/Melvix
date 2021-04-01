// MIT License, Copyright (c) 2020 Marvin Borner

#include <errno.h>
#include <fs.h>
#include <load.h>
#include <mem.h>
#include <mm.h>
#include <str.h>

#define PROC_STACK_SIZE 0x4000

res elf_load(const char *path, struct proc *proc)
{
	if (!memory_valid(path))
		return -EFAULT;

	struct stat s = { 0 };
	memory_bypass_enable();
	res stat = vfs_stat(path, &s);
	memory_bypass_disable();
	if (stat != 0)
		return stat;

	struct elf_header header = { 0 };
	memory_bypass_enable();
	res read = vfs_read(path, &header, 0, sizeof(header));
	memory_bypass_disable();
	if (read < 0)
		return read;
	if (read != sizeof(header))
		return -ENOEXEC;

	strlcpy(proc->name, path, sizeof(proc->name));

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
		memory_bypass_enable();
		if (vfs_read(path, &program, header.phoff + header.phentsize * i,
			     sizeof(program)) != sizeof(program)) {
			memory_bypass_disable();
			return -ENOEXEC;
		}
		memory_bypass_disable();

		if (program.vaddr == 0)
			continue;

		if (!memory_is_user(program.vaddr))
			return -ENOEXEC;

		struct page_dir *prev;
		memory_backup_dir(&prev);
		memory_switch_dir(proc->page_dir);

		struct memory_range vrange = memory_range_around(program.vaddr, program.memsz);
		struct memory_range prange = physical_alloc(vrange.size);
		virtual_map(proc->page_dir, prange, vrange.base, MEMORY_CLEAR | MEMORY_USER);

		memory_bypass_enable();
		if ((u32)vfs_read(proc->name, (void *)program.vaddr, program.offset,
				  program.filesz) != program.filesz) {
			memory_bypass_disable();
			memory_switch_dir(prev);
			return -ENOEXEC;
		}
		memory_bypass_disable();

		memory_switch_dir(prev);
	}

	struct page_dir *prev;
	memory_backup_dir(&prev);
	memory_switch_dir(proc->page_dir);

	u32 stack = (u32)memory_alloc(proc->page_dir, PROC_STACK_SIZE, MEMORY_USER | MEMORY_CLEAR);
	proc->regs.ebp = stack + PROC_STACK_SIZE;
	proc->regs.useresp = stack + PROC_STACK_SIZE;
	proc->regs.eip = header.entry;
	proc->entry = header.entry;

	memory_switch_dir(prev);
	return EOK;
}
