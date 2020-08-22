// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <cpu.h>
#include <def.h>
#include <fs.h>
#include <load.h>
#include <mem.h>
#include <proc.h>
#include <str.h>

int bin_load(char *path, struct proc *proc)
{
	char *data = read_file(path);
	if (!data)
		return 1;

	u32 stack = (u32)malloc(0x2000) + 0x1000;

	proc->regs.ebp = (u32)stack;
	proc->regs.useresp = (u32)stack;
	proc->regs.eip = (u32)data;
	strcpy(proc->name, path + 1);
	return 0;
}

int elf_verify(struct elf_header *h)
{
	return h->ident[0] == ELF_MAG && (strncmp((char *)&h->ident[1], "ELF", 3) == 0) &&
	       h->ident[4] == ELF_32 && h->ident[5] == ELF_LITTLE && h->ident[6] == ELF_CURRENT &&
	       h->machine == ELF_386 && (h->type == ET_REL || h->type == ET_EXEC);
}

// TODO: Fix elf loading
void elf_load(char *path, struct proc *proc)
{
	char *data = read_file(path);
	struct elf_header *h = (struct elf_header *)data;

	if (!elf_verify(h)) {
		printf("File is not elf");
		return;
	}

	if (h->type != ET_REL)
		return;

	struct elf_program_header *phdrs = (struct elf_program_header *)((u32 *)h + h->phoff);

	for (int i = 0; i < h->phnum; i++) {
		struct elf_program_header *phdr = &phdrs[i];
		if (phdr->type != PT_LOAD)
			continue;
		memcpy((void *)phdr->vaddr, h + phdr->offset, phdr->filesz);
		memset((void *)(phdr->vaddr + phdr->filesz), phdr->memsz - phdr->filesz, 0);
	}

	u32 stack = (u32)malloc(0x1000) + 0x1000;
	proc->regs.ebp = (u32)stack;
	proc->regs.esp = (u32)stack;
	proc->regs.useresp = (u32)stack;
	proc->regs.eip = (u32)h->entry;
	strcpy(proc->name, path + 1);
}
