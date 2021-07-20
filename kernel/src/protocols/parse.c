// MIT License, Copyright (c) 2021 Marvin Borner

#include <kernel.h>
#include <protocols.h>
#include <protocols/multiboot1.h>

PROTECTED static struct boot_information info = { 0 };

CLEAR struct boot_information *protocol_convert(u32 magic, uintptr_t address)
{
	if (multiboot1_detect(magic, address))
		return multiboot1_convert(address, &info);

	kernel_panic("Invalid multiboot");
	return NULL;
}
