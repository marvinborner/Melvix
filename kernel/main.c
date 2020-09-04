// MIT License, Copyright (c) 2020 Marvin Borner

#include <boot.h>
#include <cpu.h>
#include <fpu.h>
#include <fs.h>
#include <keyboard.h>
#include <load.h>
#include <mem.h>
#include <mouse.h>
#include <serial.h>
#include <syscall.h>
#include <timer.h>

void kernel_main(struct vid_info *vid_info)
{
	heap_init(0x00f00000);

	boot_passed = vid_info;

	// Serial connection
	serial_install();
	serial_print("\nConnected.\n");

	// Install drivers
	interrupts_install();
	fpu_install();
	timer_install();
	keyboard_install();
	mouse_install();

	// Enable drivers
	sti();

	ls_root();

	syscall_init();
	proc_init();

	idle();
}
