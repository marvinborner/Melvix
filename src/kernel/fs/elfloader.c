#include <kernel/system.h>
#include <kernel/fs/elf.h>
#include <kernel/lib/stdio.h>
#include <kernel/memory/alloc.h>
#include <kernel/lib/lib.h>
#include <kernel/memory/paging.h>
#include "load.h"

int elf_probe(uint8_t *buf)
{
	elf_header_t *header = (elf_header_t *)buf;
	if (header->ident[0] == 0x7f && header->ident[1] == 'E' && header->ident[2] == 'L' &&
	    header->ident[3] == 'F') {
		serial_printf("Buffer is valid ELF file!");
		return 1;
	}
	return 0;
}

uint8_t elf_start(uint8_t *buf)
{
	elf_header_t *header = (elf_header_t *)buf;
	serial_printf("Type: %s%s%s", header->ident[4] == 1 ? "32bit " : "64 bit",
		      header->ident[5] == 1 ? "Little Endian " : "Big endian ",
		      header->ident[6] == 1 ? "True ELF " : "buggy ELF ");
	if (header->type != 2) {
		serial_printf("File is not executable!");
		return 0;
	}

	uint32_t phys_loc = loader_get_unused_load_location();
	elf_program_header_t *ph = (elf_program_header_t *)(buf + header->phoff);
	for (int i = 0; i < header->phnum; i++, ph++) {
		switch (ph->type) {
		case 0:
			break;
		case 1:
			serial_printf(
				"LOAD: offset 0x%x vaddr 0x%x paddr 0x%x filesz 0x%x memsz 0x%x",
				ph->offset, ph->vaddr, ph->paddr, ph->filesz, ph->memsz);
			paging_map(phys_loc, ph->vaddr, PT_PRESENT | PT_RW | PT_USED);
			halt_loop();
			memcpy((void *)ph->vaddr, buf + ph->offset, ph->filesz);
			break;
		default:
			serial_printf("Unsupported type! Bail out!");
			return 0;
		}
	}

	return 0;
}

void elf_init()
{
	loader_t *elfloader = (loader_t *)kmalloc(sizeof(loader_t));
	elfloader->name = "ELF32";
	elfloader->probe = (void *)elf_probe;
	elfloader->start = (void *)elf_start;
	register_loader(elfloader);
}
