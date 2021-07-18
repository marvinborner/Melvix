// MIT License, Copyright (c) 2021 Marvin Borner

#include <protocols/multiboot1.h>

CLEAR u8 multiboot1_detect(u32 magic, uintptr_t address)
{
	UNUSED(address);
	return magic == MULTIBOOT1_MAGIC;
}

CLEAR struct boot_information *multiboot1_convert(uintptr_t address, struct boot_information *info)
{
	struct multiboot1_info *multiboot = (struct multiboot1_info *)address;

	if (multiboot->flags & MULTIBOOT1_INFO_MEMORY)
		info->memory.total = multiboot->memory_upper - multiboot->memory_lower;

	if (multiboot->flags & MULTIBOOT1_INFO_MEMORY_MAP) {
		info->memory.map.available = 1;
		info->memory.map.entries = (void *)multiboot->mmap_addr; // TODO: Convert
		info->memory.map.count = multiboot->mmap_length;
	}

	if (multiboot->flags & MULTIBOOT1_INFO_FRAMEBUFFER_INFO) {
		info->framebuffer.available = 1;
		info->framebuffer.address = multiboot->framebuffer_addr_low;
		info->framebuffer.width = multiboot->framebuffer_width;
		info->framebuffer.height = multiboot->framebuffer_height;
		info->framebuffer.pitch = multiboot->framebuffer_pitch;
		info->framebuffer.bpp = multiboot->framebuffer_bpp;
	}

	return info;
}
