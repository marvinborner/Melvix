#include <kernel/system.h>
#include <kernel/fs/elf.h>
#include <kernel/lib/stdio.h>
#include <kernel/memory/alloc.h>
#include <kernel/lib/lib.h>
#include <kernel/memory/paging.h>

#define USER_OFFSET 0x40000000
#define USER_STACK 0xF0000000
#define PT_LOAD 0x1

int is_elf(char *elf_data)
{
	elf_header_t *header = (elf_header_t *)elf_data;
	if (header->ident[0] == 0x7f && header->ident[1] == 'E' && header->ident[2] == 'L' &&
	    header->ident[3] == 'F') {
		log("Buffer is valid ELF file!");
		return 1;
	}
	return 0;
}

uint32_t load_elf(char *elf_data)
{
	uint32_t v_begin, v_end;
	elf_header_t *hdr;
	elf_program_header_t *p_entry;
	elf_section_header_t *s_entry;

	hdr = (elf_header_t *)elf_data;
	p_entry = (elf_program_header_t *)(elf_data + hdr->phoff);

	s_entry = (elf_section_header_t *)(elf_data + hdr->shoff);

	if (is_elf(elf_data) == 0)
		return 0;

	for (int pe = 0; pe < hdr->phnum; pe++, p_entry++) {
		if (p_entry->type == PT_LOAD) {
			v_begin = p_entry->vaddr;
			v_end = p_entry->vaddr + p_entry->memsz;
			if (v_begin < USER_OFFSET) {
				warn("load_elf(): can't load executable below %x\n", USER_OFFSET);
				return 0;
			}

			if (v_end > USER_STACK) {
				warn("load_elf(): can't load executable above %x\n", USER_STACK);
				return 0;
			}

			printf("ELF: entry flags: %x (%d)\n", p_entry->flags, p_entry->flags);

			memcpy((uint8_t *)v_begin, (uint8_t *)(elf_data + p_entry->offset),
			       p_entry->filesz);
			if (p_entry->memsz > p_entry->filesz) {
				char *p = (char *)p_entry->vaddr;
				for (int i = p_entry->filesz; i < (int)(p_entry->memsz); i++) {
					p[i] = 0;
				}
			}
		}
	}

	return hdr->entry;
}