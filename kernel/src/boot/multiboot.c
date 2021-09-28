// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>

#include <boot/abstract.h>
#include <boot/multiboot.h>

int multiboot_main(u32 magic, u32 addr, u32 esp)
{
	UNUSED(esp);

	assert(magic == MULTIBOOT_MAGIC);

	struct multiboot_info *info = (void *)addr;

	if (info->flags & MULTIBOOT_INFO_CMDLINE)
		abstract_boot_arguments_set((char *)info->cmdline);

	if (info->flags & MULTIBOOT_INFO_VBE_INFO)
		abstract_boot_vbe_info_set((void *)info->vbe_mode_info);

	if (info->flags & MULTIBOOT_INFO_MEM_MAP)
		abstract_boot_memory_map_set((void *)info->mmap_addr, info->mmap_length);

	return kernel_main();
}
