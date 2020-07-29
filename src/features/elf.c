#include <assert.h>
#include <def.h>
#include <elf.h>
#include <fs.h>
#include <mem.h>

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
		memcpy((void *)p->paddr, (void *)((u32)data + p->offset), p->filesz);
		p++;
	}

	void (*entry)();
	entry = (void (*)())(h->entry - offset);

	entry();
}
