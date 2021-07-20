// MIT License, Copyright(c) 2021 Marvin Borner

#include <acpi/main.h>
#include <arch.h>
#include <drivers/logger.h>
#include <drivers/vga.h>
#include <kernel.h>
#include <tasking/task.h>

static void kernel_task(void)
{
	while (1)
		;
}

void kernel_panic(const char *reason)
{
	vga_clear();
	vga_print("\n[FATAL] Kernel panic!\nReason: ");
	vga_print(reason);
	arch_halt();
}

void kernel_main(struct boot_information *info)
{
	vga_clear();

	struct task *kernel = task_create(NULL, "kernel", (uintptr_t)&kernel_task, PRIV_SUPER);
	struct task *idle = task_create(kernel, "idle", (uintptr_t)&kernel_task, PRIV_DEFAULT);

	logger_init();
	acpi_probe(info);

	while (1)
		;
}
