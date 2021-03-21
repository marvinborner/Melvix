// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef LOAD_H
#define LOAD_H

#include <def.h>
#include <proc.h>

/**
 * ELF
 */

#define ELF_MAG0 0x7F
#define ELF_MAG1 'E'
#define ELF_MAG2 'L'
#define ELF_MAG3 'F'

#define ELF_IDENT_COUNT 16
#define ELF_IDENT_MAG0 0
#define ELF_IDENT_MAG1 1
#define ELF_IDENT_MAG2 2
#define ELF_IDENT_MAG3 3

#define ELF_IDENT_CLASS 4
#define ELF_IDENT_CLASS_NONE 0
#define ELF_IDENT_CLASS_32 1
#define ELF_IDENT_CLASS_64 2

#define ELF_IDENT_DATA 5
#define ELF_IDENT_DATA_NONE 0
#define ELF_IDENT_DATA_LSB 1
#define ELF_IDENT_DATA_MSB 2

#define ELF_IDENT_VERSION 6
#define ELF_IDENT_OSABI 7
#define ELF_IDENT_ABIVERSION 8
#define ELF_IDENT_PAD 9

#define ELF_ETYPE_NONE 0
#define ELF_ETYPE_REL 1
#define ELF_ETYPE_EXEC 2
#define ELF_ETYPE_DYN 3
#define ELF_ETYPE_CORE 4

#define ELF_MACHINE_NONE 0
#define ELF_MACHINE_SPARC 2
#define ELF_MACHINE_386 3
#define ELF_MACHINE_SPARC32PLUS 18
#define ELF_MACHINE_SPARCV9 43
#define ELF_MACHINE_AMD64 62

#define ELF_FLAG_SPARC_EXT_MASK 0xffff00
#define ELF_FLAG_SPARC_32PLUS 0x000100
#define ELF_FLAG_SPARC_SUN_US1 0x000200
#define ELF_FLAG_SPARC_HAL_R1 0x000400
#define ELF_FLAG_SPARC_SUN_US3 0x000800
#define ELF_FLAG_SPARCV9_MM 0x3
#define ELF_FLAG_SPARCV9_TSO 0x0
#define ELF_FLAG_SPARCV9_PSO 0x1
#define ELF_FLAG_SPARCV9_RMO 0x2

#define ELF_PROGRAM_X 0x1
#define ELF_PROGRAM_W 0x2
#define ELF_PROGRAM_R 0x4

#define ELF_SECTION_TYPE_NULL 0
#define ELF_SECTION_TYPE_PROGBITS 1
#define ELF_SECTION_TYPE_SYMTAB 2
#define ELF_SECTION_TYPE_STRTAB 3
#define ELF_SECTION_TYPE_RELA 4
#define ELF_SECTION_TYPE_HASH 5
#define ELF_SECTION_TYPE_DYNAMIC 6
#define ELF_SECTION_TYPE_NOTE 7
#define ELF_SECTION_TYPE_NOBITS 8
#define ELF_SECTION_TYPE_REL 9
#define ELF_SECTION_TYPE_SHLIB 10
#define ELF_SECTION_TYPE_DYNSYM 11
#define ELF_SECTION_TYPE_COUNT 12

struct PACKED elf_header {
	u8 ident[ELF_IDENT_COUNT];
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

struct elf_program {
	u32 type;
	u32 offset;
	u32 vaddr;
	u32 paddr;
	u32 filesz;
	u32 memsz;
	u32 flags;
	u32 align;
};

res elf_load(const char *path, struct proc *proc);

#endif
