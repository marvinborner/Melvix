#include <stdint.h>
#include <stddef.h>
#include <kernel/system.h>
#include <kernel/fs/elf.h>
#include <kernel/lib/stdio.h>
#include <kernel/memory/alloc.h>
#include <kernel/lib/lib.h>
#include <kernel/memory/paging.h>
#include <kernel/fs/ext2.h>
#include <kernel/gdt/gdt.h>
#include <kernel/tasks/process.h>

int is_elf(struct elf_header *header)
{
	if (header->ident[0] == ELF_MAG && header->ident[1] == 'E' && header->ident[2] == 'L' &&
	    header->ident[3] == 'F' && header->ident[4] == ELF_32 &&
	    header->ident[5] == ELF_LITTLE && header->ident[6] == ELF_CURRENT &&
	    header->machine == ELF_386 && (header->type == ET_REL || header->type == ET_EXEC)) {
		return 1;
	}
	return 0;
}

struct process *elf_load(char *path)
{
	uint8_t *file = read_file(path);
	if (!file) {
		warn("File or directory not found: %s", file);
		return NULL;
	}

	struct elf_header *header = (struct elf_header *)file;
	struct elf_program_header *program_header = (void *)header + header->phoff;

	if (!is_elf(header)) {
		warn("File not valid: %s", path);
		return NULL;
	} else {
		debug("File is valid: %s", path);
	}

	struct process *proc = process_make_new();
	proc->name = "TEST";
	proc->registers.eip = header->entry;
	paging_switch_directory(proc->cr3);
	uint32_t stk = (uint32_t)kmalloc_a(PAGE_S);
	proc->registers.useresp = 0x40000000 - (PAGE_S / 2);
	proc->registers.ebp = proc->registers.useresp;
	proc->registers.esp = proc->registers.useresp;
	paging_map_user(proc->cr3, stk, 0x40000000 - PAGE_S);

	for (int i = 0; i < header->phnum; i++, program_header++) {
		switch (program_header->type) {
		case 0:
			break;
		case 1:
			debug("Allocating space for ELF binary section...");
			uint32_t loc = (uint32_t)kmalloc_a(PAGE_S);
			paging_map_user(proc->cr3, loc, program_header->vaddr);
			memcpy((void *)program_header->vaddr,
			       ((void *)((uint32_t)file) + program_header->offset),
			       program_header->filesz);
			if (program_header->filesz > PAGE_S)
				panic("ELF binary section too large");
			break;
		default:
			warn("Unknown header type");
		}
	}

	return proc;
}