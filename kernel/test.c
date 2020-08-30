// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <boot.h>
#include <cpu.h>
#include <fs.h>
#include <keyboard.h>
#include <load.h>
#include <mem.h>
#include <mouse.h>
#include <serial.h>
#include <syscall.h>
#include <timer.h>

void test_all(struct vid_info *vid_info)
{
	heap_init(0x00f00000);
	boot_passed = vid_info;

	// Serial connection
	serial_install();
	serial_print("\nConnected testing.\n");
	assert(vid_info && vid_info->mode && vid_info->vbe);

	// Install drivers
	interrupts_install();
	timer_install();
	keyboard_install();
	mouse_install();

	ls_root();

	idle();
}
