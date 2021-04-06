// MIT License, Copyright (c) 2020 Marvin Borner

#include <cpu.h>
#include <errno.h>
#include <fs.h>
#include <load.h>
#include <mem.h>
#include <mm.h>
#include <random.h>
#include <str.h>

res elf_load(const char *name, struct proc *proc)
{
	if (!memory_valid(name))
		return -EFAULT;

	stac();
	char path[64] = { "/apps/" };
	strlcat(path, name, sizeof(path));
	strlcpy(proc->dir, path, sizeof(proc->dir));
	strlcat(path, "/exec", sizeof(path));
	strlcpy(proc->name, name, sizeof(proc->name));
	clac();

	struct stat s = { 0 };
	memory_bypass_enable();
	res stat = vfs_stat(path, &s);
	memory_bypass_disable();
	if (stat != EOK)
		return stat;

	struct elf_header header = { 0 };
	stac();
	memory_bypass_enable();
	res read = vfs_read(path, &header, 0, sizeof(header));
	memory_bypass_disable();
	clac();
	if (read < 0)
		return read;
	if (read != sizeof(header))
		return -ENOEXEC;

	// Valid?
	u8 *magic = header.ident;
	u8 valid_magic = magic[ELF_IDENT_MAG0] == ELF_MAG0 && magic[ELF_IDENT_MAG1] == ELF_MAG1 &&
			 magic[ELF_IDENT_MAG2] == ELF_MAG2 && magic[ELF_IDENT_MAG3] == ELF_MAG3 &&
			 magic[ELF_IDENT_CLASS] == ELF_IDENT_CLASS_32 &&
			 magic[ELF_IDENT_DATA] == ELF_IDENT_DATA_LSB;
	if (!valid_magic ||
	    (header.type != ELF_ETYPE_REL && header.type != ELF_ETYPE_EXEC &&
	     header.type != ELF_ETYPE_DYN) ||
	    header.version != 1 || header.machine != ELF_MACHINE_386)
		return -ENOEXEC;

	if (!memory_valid((void *)header.entry))
		return -ENOEXEC;

	// ASLR
	u32 rand_off = header.type == ELF_ETYPE_DYN ? (rand() & 0xffff) * PAGE_SIZE : 0;

	// Loop through programs
	for (u32 i = 0; i < header.phnum; i++) {
		struct elf_program program = { 0 };

		memory_bypass_enable();
		if (vfs_read(path, &program, header.phoff + header.phentsize * i,
			     sizeof(program)) != sizeof(program)) {
			memory_bypass_disable();
			clac();
			return -ENOEXEC;
		}
		memory_bypass_disable();

		if (program.type == ELF_PROGRAM_TYPE_INTERP)
			return -ENOEXEC;

		if (program.vaddr == 0 || program.type != ELF_PROGRAM_TYPE_LOAD)
			continue;

		if (!memory_is_user(program.vaddr))
			return -ENOEXEC;

		struct page_dir *prev;
		memory_backup_dir(&prev);
		memory_switch_dir(proc->page_dir);

		struct memory_range vrange =
			memory_range_around(program.vaddr + rand_off, program.memsz);
		struct memory_range prange = physical_alloc(vrange.size);
		virtual_map(proc->page_dir, prange, vrange.base, MEMORY_CLEAR | MEMORY_USER);

		memory_bypass_enable();
		if ((u32)vfs_read(path, (void *)((u32)program.vaddr + rand_off), program.offset,
				  program.filesz) != program.filesz) {
			memory_bypass_disable();
			memory_switch_dir(prev);
			return -ENOEXEC;
		}
		memory_bypass_disable();

		memory_switch_dir(prev);
	}

	// Find section string table
	struct elf_section section_strings = { 0 };
	memory_bypass_enable();
	if (vfs_read(path, &section_strings, header.shoff + header.shentsize * header.shstrndx,
		     sizeof(section_strings)) != sizeof(section_strings)) {
		memory_bypass_disable();
		clac();
		return -ENOEXEC;
	}
	memory_bypass_disable();

	if (section_strings.type != ELF_SECTION_TYPE_STRTAB)
		return -ENOEXEC;

	// Loop through sections
	for (u32 i = 0; i < header.shnum; i++) {
		struct elf_section section = { 0 };
		memory_bypass_enable();
		if (vfs_read(path, &section, header.shoff + header.shentsize * i,
			     sizeof(section)) != sizeof(section)) {
			memory_bypass_disable();
			clac();
			return -ENOEXEC;
		}
		memory_bypass_disable();

		// TODO: Use section and symbol name for logging or something? (e.g. in page fault handler)
		/* u32 offset = section_strings.offset + section.name; */
		/* if (offset >= s.size) */
		/* 	return -ENOEXEC; */
		/* memory_bypass_enable(); */
		/* char name[64] = { 0 }; // Max length? */
		/* if (vfs_read(path, &name, offset, sizeof(name)) != sizeof(name)) { */
		/* 	memory_bypass_disable(); */
		/* 	clac(); */
		/* 	return -ENOEXEC; */
		/* } */
		/* memory_bypass_disable(); */
		/* printf("%d\n", section.name); */
		/* if (section.type == ELF_SECTION_TYPE_SYMTAB) { */
		/* } else if (section.type == ELF_SECTION_TYPE_STRTAB && i != header.shstrndx) { */
		/* } */

		// Remap readonly sections
		if (!(section.flags & ELF_SECTION_FLAG_WRITE)) {
			struct memory_range range =
				memory_range_around(section.addr + rand_off, section.size);
			virtual_remap_readonly(proc->page_dir, range);
		}
	}

	struct page_dir *prev;
	memory_backup_dir(&prev);
	memory_switch_dir(proc->page_dir);

	stac();

	// Allocate stack with readonly lower and upper page boundary
	u32 stack = PAGE_SIZE + (u32)memory_alloc(proc->page_dir, PROC_STACK_SIZE + 2 * PAGE_SIZE,
						  MEMORY_USER | MEMORY_CLEAR);
	virtual_remap_readonly(proc->page_dir, memory_range(stack - PAGE_SIZE, PAGE_SIZE));
	virtual_remap_readonly(proc->page_dir, memory_range(stack + PROC_STACK_SIZE, PAGE_SIZE));

	proc->regs.ebp = stack + PROC_STACK_SIZE;
	proc->regs.useresp = stack + PROC_STACK_SIZE;
	proc->regs.eip = header.entry + rand_off;
	proc->entry = header.entry + rand_off;

	clac();

	memory_switch_dir(prev);
	return EOK;
}
