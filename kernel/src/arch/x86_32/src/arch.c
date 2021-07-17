// MIT License, Copyright (c) 2021 Marvin Borner

#include <stdint.h>

#include <arch.h>
#include <gdt.h>
#include <idt.h>
#include <kernel.h>
#include <protocols.h>

void arch_init(u32 magic, uintptr_t addr);
CLEAR void arch_init(u32 magic, uintptr_t addr)
{
	gdt_init();
	idt_init();

	struct boot_information *info = protocol_convert(magic, addr);

	kernel_main(info);
}
