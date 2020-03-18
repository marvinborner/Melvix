#include <kernel/fs/elf.h>
#include <kernel/lib/stdio.h>
#include <kernel/memory/alloc.h>
#include <kernel/lib/lib.h>
#include <kernel/memory/paging.h>
#include "load.h"

elf_priv_data *elf_probe(uint8_t *buf)
{
	elf_header_t *header = (elf_header_t *)buf;
	/* The first four bytes are 0x7f and 'ELF' */
	if (header->e_ident[0] == 0x7f && header->e_ident[1] == 'E' && header->e_ident[2] == 'L' &&
	    header->e_ident[3] == 'F') {
		/* Valid ELF! */
		serial_printf("Buffer is valid ELF file!");
		return (void *)1;
	}
	return 0;
}

uint8_t elf_start(uint8_t *buf)
{
	elf_header_t *header = (elf_header_t *)buf;
	serial_printf("Type: %s%s%s", header->e_ident[4] == 1 ? "32bit " : "64 bit",
		      header->e_ident[5] == 1 ? "Little Endian " : "Big endian ",
		      header->e_ident[6] == 1 ? "True ELF " : "buggy ELF ");
	if (header->e_type != 2) {
		serial_printf("File is not executable!");
		return 0;
	}
	/* Map the virtual space */
	uint32_t phys_loc = loader_get_unused_load_location();
	uint32_t cr3 = kmalloc(4096);
	/* Find first program header and loop through them */
	elf_program_header_t *ph = (elf_program_header_t *)(buf + header->e_phoff);
	for (int i = 0; i < header->e_phnum; i++, ph++) {
		switch (ph->p_type) {
		case 0: /* NULL */
			break;
		case 1: /* LOAD */
			serial_printf(
				"LOAD: offset 0x%x vaddr 0x%x paddr 0x%x filesz 0x%x memsz 0x%x",
				ph->p_offset, ph->p_vaddr, ph->p_paddr, ph->p_filesz, ph->p_memsz);
			paging_map(phys_loc, ph->p_vaddr, PT_PRESENT | PT_RW | PT_USED);
			memcpy(ph->p_vaddr, buf + ph->p_offset, ph->p_filesz);
			break;
		default: /* @TODO add more */
			serial_printf("Unsupported p_type! Bail out!");
			return 0;
		}
	}

	return 0; //START("elf32", header->e_entry);
}

void elf_init()
{
	loader_t *elfloader = (loader_t *)kmalloc(sizeof(loader_t));
	elfloader->name = "ELF32";
	elfloader->probe = (void *)elf_probe;
	elfloader->start = (void *)elf_start;
	register_loader(elfloader);
	// _kill();
}