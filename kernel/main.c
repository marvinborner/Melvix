// MIT License, Copyright (c) 2020 Marvin Borner

#include <boot.h>
#include <cpu.h>
#include <fb.h>
#include <fs.h>
#include <ide.h>
#include <interrupts.h>
#include <keyboard.h>
#include <load.h>
#include <mem.h>
#include <mm.h>
#include <mouse.h>
#include <net.h>
#include <pci.h>
#include <random.h>
#include <serial.h>
#include <syscall.h>
#include <timer.h>

void kernel_main(struct mem_info *mem_info, struct vid_info *vid_info); // Decl
void kernel_main(struct mem_info *mem_info, struct vid_info *vid_info)
{
	// Serial connection
	serial_install();
	serial_print("\nKernel was compiled at " __TIME__ " on " __DATE__ "\n");
	serial_print("Serial connected.\n");

	memory_install(mem_info, vid_info);
	memory_switch_dir(virtual_kernel_dir());
	paging_enable();

	cpu_enable_features();
	cpu_print();
	srand(rdseed());

	// Install drivers
	vfs_install();
	device_install();
	ata_install();
	pci_install();
	interrupts_install();
	timer_install();
	keyboard_install();
	mouse_install();
	fb_install(vid_info);
	/* net_install(); */

	// Enable drivers
	sti();
	keyboard_reset();

	syscall_init();
	proc_init();

	idle();
}
