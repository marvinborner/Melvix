#include <assert.h>
#include <def.h>
#include <elf.h>
#include <fs.h>
#include <mem.h>
#include <str.h>

int elf_verify(struct elf_header *h)
{
	return (h->ident[0] == ELF_MAG && !strncmp((char *)&h->ident[1], "ELF", 3) &&
		h->ident[4] == ELF_32 && h->ident[5] == ELF_LITTLE && h->ident[6] == ELF_CURRENT &&
		h->machine == ELF_386 && (h->type == ET_REL || h->type == ET_EXEC));
}

void elf_load(char *path)
{
	u32 *data = read_file(path);

	struct elf_header *h = (struct elf_header *)data;
	assert(elf_verify(h));

	struct elf_program_header *p = (struct elf_program_header *)((u32)data + h->phoff);
	struct elf_program_header *p_end =
		(struct elf_program_header *)((u32)p + (h->phentsize * h->phnum));

	u32 offset = (p->vaddr - p->paddr);
	while (p < p_end) {
		printf("\nheader: 0x%x\n", p->paddr);
		printf("filesz: %d\n", p->filesz);
		/* memcpy(p->paddr, (u32)data + p->offset, p->filesz); */
		memcpy((u32 *)p->paddr, (u32 *)((u32)data + p->offset), p->filesz);
		p++;
	}

	void (*entry)();
	entry = (void (*)())(h->entry - offset);

	entry();
}
