/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <boot/abstract.h>
#include <core/descriptors/global.h>
#include <core/descriptors/interrupt.h>
#include <drivers/install.h>
#include <drivers/timer.h>
#include <management/disk/index.h>

PROTECTED extern u32 __stack_chk_guard;
PROTECTED u32 __stack_chk_guard;

NORETURN void kernel_main(void)
{
	__stack_chk_guard = 0x42424242; // TODO: Random

	abstract_boot_finish();
	gdt_init();
	idt_init();

	drivers_install();

	device_request(DEVICE_TIMER, DEVICE_TIMER_PHASE, 1000);

	disk_iterate(disk_load);

	__asm__ volatile("sti");
	while (1)
		;
}
