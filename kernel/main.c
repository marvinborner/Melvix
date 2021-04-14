// MIT License, Copyright (c) 2020 Marvin Borner

#include <boot.h>
#include <cpu.h>
#include <fs.h>
#include <ide.h>
#include <interrupts.h>
#include <io.h>
#include <load.h>
#include <mem.h>
#include <mm.h>
#include <net.h>
#include <pci.h>
#include <rand.h>
#include <rtc.h>
#include <serial.h>
#include <syscall.h>

PROTECTED extern u32 __stack_chk_guard;
PROTECTED u32 __stack_chk_guard;

PROTECTED u32 tss_entry = 0;
PROTECTED u32 boot_drive = 0;

int kernel_main(struct boot_info *boot); // Decl
int kernel_main(struct boot_info *boot)
{
	// Serial connection
	serial_install();
	serial_print("\nKernel was compiled at " __TIME__ " on " __DATE__ "\n");
	serial_print("Serial connected.\n");

	tss_entry = boot->tss;
	boot_drive = boot->drive;

	memory_install(boot);
	memory_switch_dir(virtual_kernel_dir());

	cpu_enable_features();
	cpu_print();

	srand(rtc_stamp());
	__stack_chk_guard = rand();

	// Install drivers
	vfs_install();
	ata_install();
	pci_install();
	interrupts_install();
	io_install(boot);

	// Enable drivers
	sti();

	syscall_init();

	proc_init();

	return 1;
}
