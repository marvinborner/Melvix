// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <def.h>
#include <multiboot.h>

static struct multiboot_info *info = NULL;

void multiboot_init(u32 magic, u32 addr)
{
	assert(magic == MULTIBOOT_MAGIC);
	info = (void *)addr;

	if (info->flags & MULTIBOOT_INFO_CMDLINE) {
		printf("CMDLINE: '%s'\n", info->cmdline);
	}
}
