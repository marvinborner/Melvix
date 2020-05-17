#include <fs/elf.h>
#include <fs/ext2.h>
#include <gdt/gdt.h>
#include <io/io.h>
#include <lib/lib.h>
#include <lib/stdio.h>
#include <lib/stdlib.h>
#include <memory/alloc.h>
#include <memory/paging.h>
#include <stddef.h>
#include <stdint.h>
#include <system.h>
#include <tasks/process.h>

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
	struct page_dir *prev_dir;
	if (current_proc)
		prev_dir = current_proc->cr3;
	else
		prev_dir = current_page_directory;

	u8 *file = read_file(path);
	if (!file) {
		warn("File or directory not found: %s", path);
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
	strcpy(proc->name, path);
	proc->registers.eip = header->entry;

	log("1");
	paging_switch_directory(proc->cr3);
	log("2");
	u32 stk = (u32)malloc(PAGE_SIZE);
	log("3");
	proc->registers.useresp = 0x40000000 - (PAGE_SIZE / 2);
	proc->registers.ebp = proc->registers.useresp;
	proc->registers.esp = proc->registers.useresp;
	paging_map(stk, 0x40000000 - PAGE_SIZE, PT_USER);

	for (int i = 0; i < header->phnum; i++, program_header++) {
		switch (program_header->type) {
		case 0:
			break;
		case 1: {
			u32 loc = (u32)malloc(PAGE_SIZE);
			paging_map(loc, program_header->vaddr, PT_USER);
			memcpy((void *)program_header->vaddr,
			       ((void *)((u32)file) + program_header->offset),
			       program_header->filesz);
			if (program_header->filesz > PAGE_SIZE)
				panic("ELF binary section too large");
			break;
		}
		default:
			warn("Unknown header type");
		}
	}

	paging_switch_directory(prev_dir);
	return proc;
}