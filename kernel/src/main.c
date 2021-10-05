/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <def.h>
#include <print.h>

#include <boot/abstract.h>
#include <core/descriptors/global.h>
#include <drivers/install.h>

#include <management/dev/sys.h>

NORETURN void kernel_main(void)
{
	abstract_boot_finish();
	gdt_init();

	drivers_install();

	/* dev_write(DEV_LOGGER, "Hallo", 0, 5); */

	log("Hi");

	while (1)
		;
}
