// MIT License, Copyright (c) 2021 Marvin Borner

#include <stdint.h>

#include <arch.h>
#include <gdt.h>
#include <interrupts/idt.h>
#include <interrupts/pic.h>
#include <protocols.h>

NORETURN void arch_halt(void)
{
	__asm__ volatile("cli");
	while (1)
		__asm__ volatile("hlt");
}

void arch_init(u32 magic, uintptr_t addr);
CLEAR void arch_init(u32 magic, uintptr_t addr)
{
	gdt_init();
	pic_init();
	idt_init();

	struct boot_information *info = protocol_convert(magic, addr);

	kernel_main(info);
}
