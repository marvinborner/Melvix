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

int elf_verify(struct elf_header *header)
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
	log("ELF START");

	u8 *file = read_file(path);
	if (!file) {
		warn("File or directory not found: %s", path);
		return NULL;
	}

	struct elf_header *header = (struct elf_header *)file;

	if (!elf_verify(header)) {
		warn("File not valid: %s", path);
		return NULL;
	} else {
		debug("File is valid: %s", path);
	}

	struct process *proc = process_make_new();
	paging_switch_dir(proc->cr3);

	u32 image_low = 0xFFFFFFFF;
	u32 image_high = 0;

	// Parse ELF
	u32 j = 0;
	for (u32 i = 0; i < header->shentsize * header->shnum; i += header->shentsize) {
		struct elf_section_header *sh = (void *)header + (header->shoff + i);
		if (sh->addr != 0) {
			log("%x", sh->addr);
			/* for (u32 j = 0; j < sh->size; j += PAGE_SIZE) { */
			/* 	paging_frame_alloc(paging_get_page(sh->addr + j, proc->cr3)); */
			/* 	invlpg(sh->addr + j); */
			/* } */
			while (j < sh->size) {
				paging_frame_alloc(paging_get_page(sh->addr + j, proc->cr3));
				invlpg(sh->addr + j);
				j += 0x1000;
			}

			if (sh->type == 8) // Is .bss
				memset(sh->addr, 0, sh->size);
			else
				memcpy(sh->addr, header + sh->offset, sh->size);

			if (sh->addr < image_low)
				image_low = sh->addr;

			if (sh->addr + sh->size > image_high)
				image_high = sh->addr + sh->size;
		}
	}

	// Stack
	struct page_table_entry *stack_page = paging_get_page(USER_STACK_LOW, proc->cr3);
	paging_frame_alloc(stack_page);
	stack_page->writable = 1;
	invlpg(USER_STACK_LOW);

	strcpy(proc->name, path);
	proc->brk = image_high;
	proc->regs.useresp = USER_STACK_HIGH;
	proc->regs.ebp = proc->regs.useresp;
	proc->regs.esp = proc->regs.useresp;
	proc->regs.eip = header->entry;

	return proc;
}