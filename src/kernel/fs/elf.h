#ifndef MELVIX_ELF_H
#define MELVIX_ELF_H

#include <stdint.h>
#include <tasks/process.h>

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
	u32 sig;
};

struct elf_header {
	u8 ident[16];
	u16 type;
	u16 machine;
	u32 version;
	u32 entry;
	u32 phoff;
	u32 shoff;
	u32 flags;
	u16 ehsize;
	u16 phentsize;
	u16 phnum;
	u16 shentsize;
	u16 shnum;
	u16 shstrndx;
};

struct elf_section_header {
	u32 name;
	u32 type;
	u32 flags;
	u32 addr;
	u32 offset;
	u32 size;
	u32 link;
	u32 info;
	u32 addralign;
	u32 entsize;
};

struct elf_program_header {
	u32 type;
	u32 offset;
	u32 vaddr;
	u32 paddr;
	u32 filesz;
	u32 memsz;
	u32 flags;
	u32 align;
};

int is_elf(struct elf_header *header);
struct process *elf_load(char *path);

#endif