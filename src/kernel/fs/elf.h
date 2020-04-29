#ifndef MELVIX_ELF_H
#define MELVIX_ELF_H

#include <stdint.h>
#include <kernel/tasks/process.h>

#define ELF_MAG 0x7F // 0
#define ELF_32 (1) // 4: 32-bit Architecture
#define ELF_LITTLE (1) // 5: Little Endian
#define ELF_CURRENT (1) // 6: ELF Current Version
#define ELF_386 (3) // header->machine x86 machine type

#define ET_NONE 0 // Unkown type
#define ET_REL 1 // Relocatable file
#define ET_EXEC 2 // Executable file

#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6
#define PT_LOPROC 0x70000000
#define PT_HIPROC 0x7fffffff

#define PF_X 0x1
#define PF_W 0x2
#define PF_R 0x4

struct elf_priv_data {
	uint32_t sig;
};

struct elf_header {
	uint8_t ident[16];
	uint16_t type;
	uint16_t machine;
	uint32_t version;
	uint32_t entry;
	uint32_t phoff;
	uint32_t shoff;
	uint32_t flags;
	uint16_t ehsize;
	uint16_t phentsize;
	uint16_t phnum;
	uint16_t shentsize;
	uint16_t shnum;
	uint16_t shstrndx;
};

struct elf_section_header {
	uint32_t name;
	uint32_t type;
	uint32_t flags;
	uint32_t addr;
	uint32_t offset;
	uint32_t size;
	uint32_t link;
	uint32_t info;
	uint32_t addralign;
	uint32_t entsize;
};

struct elf_program_header {
	uint32_t type;
	uint32_t offset;
	uint32_t vaddr;
	uint32_t paddr;
	uint32_t filesz;
	uint32_t memsz;
	uint32_t flags;
	uint32_t align;
};

int is_elf(struct elf_header *header);
struct process *elf_load(char *path);

#endif