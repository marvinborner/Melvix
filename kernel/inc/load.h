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
#define ELF_ETYPE_NUM 5

#define ELF_MACHINE_NONE 0
#define ELF_MACHINE_SPARC 2
#define ELF_MACHINE_386 3
#define ELF_MACHINE_SPARC32PLUS 18
#define ELF_MACHINE_SPARCV9 43
#define ELF_MACHINE_AMD64 62

#define ELF_PROGRAM_TYPE_NULL 0
#define ELF_PROGRAM_TYPE_LOAD 1
#define ELF_PROGRAM_TYPE_DYNAMIC 2
#define ELF_PROGRAM_TYPE_INTERP 3
#define ELF_PROGRAM_TYPE_NOTE 4
#define ELF_PROGRAM_TYPE_SHLIB 5
#define ELF_PROGRAM_TYPE_PHDR 6
#define ELF_PROGRAM_TYPE_TLS 7

#define ELF_PROGRAM_FLAG_X 0x1
#define ELF_PROGRAM_FLAG_W 0x2
#define ELF_PROGRAM_FLAG_R 0x4

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

#define ELF_SECTION_FLAG_WRITE 0x1
#define ELF_SECTION_FLAG_ALLOC 0x2
#define ELF_SECTION_FLAG_EXEC 0x3
#define ELF_SECTION_FLAG_MERGE 0x10
#define ELF_SECTION_FLAG_STRINGS 0x20
#define ELF_SECTION_FLAG_INFO_LINK 0x40
#define ELF_SECTION_FLAG_LINK_ORDER 0x80
#define ELF_SECTION_FLAG_OS_SPECIAL 0x100
#define ELF_SECTION_FLAG_GROUP 0x200
#define ELF_SECTION_FLAG_TLS 0x400
#define ELF_SECTION_FLAG_COMPRESSED 0x800

#define ELF_BSS ".bss"
#define ELF_DATA ".data"
#define ELF_DEBUG ".debug"
#define ELF_DYNAMIC ".dynamic"
#define ELF_DYNSTR ".dynstr"
#define ELF_DYNSYM ".dynsym"
#define ELF_FINI ".fini"
#define ELF_GOT ".got"
#define ELF_HASH ".hash"
#define ELF_INIT ".init"
#define ELF_REL_DATA ".rel.data"
#define ELF_REL_FINI ".rel.fini"
#define ELF_REL_INIT ".rel.init"
#define ELF_REL_DYN ".rel.dyn"
#define ELF_REL_RODATA ".rel.rodata"
#define ELF_REL_TEXT ".rel.text"
#define ELF_RODATA ".rodata"
#define ELF_SHSTRTAB ".shstrtab"
#define ELF_STRTAB ".strtab"
#define ELF_SYMTAB ".symtab"
#define ELF_TEXT ".text"

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

struct PACKED elf_program {
	u32 type;
	u32 offset;
	u32 vaddr;
	u32 paddr;
	u32 filesz;
	u32 memsz;
	u32 flags;
	u32 align;
};

struct PACKED elf_section {
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

struct PACKED elf_symbol {
	u32 name;
	u32 value;
	u32 size;
	u8 info;
	u8 other;
	u16 shndx;
};

res elf_load(const char *path, struct proc *proc) NONNULL;

#endif
