// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>

#include <boot/abstract.h>
#include <core/descriptors/global.h>
#include <management/dev/detection.h>

PROTECTED extern u32 __stack_chk_guard;
PROTECTED u32 __stack_chk_guard;

int kernel_main(void)
{
	abstract_boot_finish();
	gdt_init();

	/* memory_install(); */

	/* cpu_enable_features(); */
	/* cpu_print(); */

	/* srand(rtc_stamp()); */
	/* __stack_chk_guard = rand(); */

	/* // Install drivers */
	dev_detect();

	/* syscall_init(); */
	/* proc_init(); */
	while (1)
		;

	return 1;
}
