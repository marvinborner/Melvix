// MIT License, Copyright (c) 2020 Marvin Borner

#include <dev.h>
#include <drivers/cpu.h>
#include <drivers/gdt.h>
#include <drivers/ide.h>
#include <drivers/int.h>
#include <drivers/pci.h>
#include <drivers/pic.h>
#include <drivers/rtc.h>
#include <drivers/serial.h>
#include <fs.h>
#include <load.h>
#include <mem.h>
#include <mm.h>
#include <multiboot.h>
#include <rand.h>
#include <syscall.h>

PROTECTED extern u32 __stack_chk_guard;
PROTECTED u32 __stack_chk_guard;

int kernel_main(u32 magic, u32 addr, u32 esp); // Decl
int kernel_main(u32 magic, u32 addr, u32 esp)
{
	serial_install();

	gdt_install(esp);
	multiboot_init(magic, addr);

	memory_install();

	cpu_enable_features();
	cpu_print();

	srand(rtc_stamp());
	__stack_chk_guard = rand();

	// Install drivers
	vfs_install();
	ata_install();
	pci_install();
	pic_install();
	idt_install();
	dev_install();

	syscall_init();
	proc_init();

	return 1;
}
