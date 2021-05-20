// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <def.h>
#include <drivers/serial.h>
#include <mem.h>
#include <mm.h>
#include <multiboot.h>

PROTECTED static struct multiboot_info *info = NULL;
PROTECTED static u8 vbe_available = 0;
PROTECTED static char vbe[256] = { 0 };

CLEAR static void multiboot_parse_cmdline(const char *line)
{
	const char *start = line;
	for (const char *p = line; p && *p; p++) {
		if (*p == ' ')
			start = p + 1;

		if (memcmp(start, "log", 3) == 0 && !ALPHANUMERIC(start[3])) {
			serial_enable();
			start += 3;
		}
	}
}

CLEAR u32 multiboot_vbe(void)
{
	return vbe_available ? (u32)vbe : 0;
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
			physical_set_free(memory_range_from(mmap->addr, mmap->len));
		} else if (mmap->type == MULTIBOOT_MEMORY_BADRAM) {
			printf("Defect memory at 0x%x-0x%x\n", mmap->addr, mmap->addr + mmap->len);
		} else {
			// Memory is set to 'used' by default
		}

		mmap++;
	}

	physical_set_total(total);
}

CLEAR void multiboot_init(u32 magic, u32 addr)
{
	assert(magic == MULTIBOOT_MAGIC);
	info = (void *)addr;

	if (info->flags & MULTIBOOT_INFO_CMDLINE)
		multiboot_parse_cmdline((const char *)info->cmdline);

	if (info->flags & MULTIBOOT_INFO_VBE_INFO) {
		memcpy(vbe, (void *)info->vbe_mode_info, sizeof(vbe));
		vbe_available = 1;
	}

	serial_print("Kernel was compiled at " __TIME__ " on " __DATE__ "\n");
}
