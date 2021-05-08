// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <def.h>
#include <mem.h>
#include <mm.h>
#include <multiboot.h>
#include <serial.h>

PROTECTED static struct multiboot_info *info = NULL;

CLEAR static void multiboot_parse_cmdline(const char *line)
{
	const char *start = line;
	for (const char *p = line; p && *p; p++) {
		if (*p == ' ')
			start = p + 1;

		if (memcmp(start, "log", 3) == 0) {
			serial_enable();
			start += 3;
		}
	}
}

CLEAR void multiboot_init(u32 magic, u32 addr)
{
	assert(magic == MULTIBOOT_MAGIC);
	info = (void *)addr;

	if (info->flags & MULTIBOOT_INFO_CMDLINE)
		multiboot_parse_cmdline((const char *)info->cmdline);
}

CLEAR u32 multiboot_vbe(void)
{
	assert(info->flags & MULTIBOOT_INFO_VBE_INFO);

	return info->vbe_mode_info;
}

CLEAR void multiboot_mmap(void)
{
	assert(info->flags & MULTIBOOT_INFO_MEM_MAP);

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
