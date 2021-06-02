// MIT License, Copyright (c) 2020 Marvin Borner

#include <drivers/cpu.h>
#include <errno.h>
#include <fs.h>
#include <load.h>
#include <mem.h>
#include <mm.h>
#include <rand.h>
#include <str.h>

res elf_load(const char *name, struct proc *proc)
{
	if (!memory_readable(name))
		return -EFAULT;

	char path[64] = { "/apps/" };
	strlcat_user(path, name, sizeof(path));
	strlcpy_user(proc->dir, path, sizeof(proc->dir));
	strlcat_user(path, "/exec", sizeof(path));
	strlcpy_user(proc->name, name, sizeof(proc->name));

	struct stat s = { 0 };
	memory_bypass_enable();
	res stat = vfs_stat(path, &s);
	memory_bypass_disable();
	if (stat != EOK)
		return stat;

	struct elf_header header = { 0 };
	memory_bypass_enable();
	stac();
	res read = vfs_read(path, &header, 0, sizeof(header));
	clac();
	memory_bypass_disable();
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

	if (!memory_is_user((void *)header.entry))
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
			return -ENOEXEC;
		}
		memory_bypass_disable();

		if (program.type == ELF_PROGRAM_TYPE_INTERP)
			return -ENOEXEC;

		if (program.vaddr == 0 || program.type != ELF_PROGRAM_TYPE_LOAD)
			continue;

		if (!memory_is_user((void *)program.vaddr))
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
			return -ENOEXEC;
		}
		memory_bypass_disable();

		// Remap readonly sections
		if (!(section.flags & ELF_SECTION_FLAG_WRITE) && section.addr &&
		    memory_is_user((void *)section.addr)) {
			struct memory_range range =
				memory_range_around(section.addr + rand_off, section.size);
			virtual_remap_readonly(proc->page_dir, range);
		}
	}

	proc->entry = header.entry + rand_off;
	proc_make_regs(proc);

	return EOK;
}
