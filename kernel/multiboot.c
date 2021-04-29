// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <def.h>
#include <mm.h>
#include <multiboot.h>

PROTECTED static struct multiboot_info *info = NULL;

CLEAR void multiboot_init(u32 magic, u32 addr)
{
	assert(magic == MULTIBOOT_MAGIC);
	info = (void *)addr;

	if (info->flags & MULTIBOOT_INFO_CMDLINE) {
		// TODO: Do something useful with grub cmdline?
		/* printf("CMDLINE: '%s'\n", info->cmdline); */
	}
}

CLEAR void multiboot_mmap(void)
{
	assert(info->flags & MULTIBOOT_INFO_MEMORY);

	struct multiboot_mmap_entry *mmap = (void *)info->mmap_addr;
	u32 length = info->mmap_length;
	u32 count = length / sizeof(*mmap);

	u32 total = 0;

	for (u32 i = 0; i < count; i++) {
		/* printf("Memory region 0x%x-0x%x\n", mmap->addr, mmap->addr + mmap->len); */
		if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
			total += mmap->len;
			physical_set_free(memory_range_around(mmap->addr, mmap->len));
		} else if (mmap->type == MULTIBOOT_MEMORY_BADRAM) {
			printf("Defect memory at 0x%x-0x%x\n", mmap->addr, mmap->addr + mmap->len);
			physical_set_used(memory_range_around(mmap->addr, mmap->len));
		} else {
			physical_set_used(memory_range_around(mmap->addr, mmap->len));
		}

		mmap++;
	}

	physical_set_total(total);
}
