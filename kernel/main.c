// MIT License, Copyright (c) 2020 Marvin Borner

#include "config.h"
#include <boot.h>
#include <cpu.h>
#include <def.h>
#include <fs.h>
#include <gui.h>
#include <interrupts.h>
#include <keyboard.h>
#include <load.h>
#include <mem.h>
#include <print.h>
#include <serial.h>
#include <syscall.h>
#include <timer.h>

void kernel_main(struct vid_info *vid_info)
{
	HEAP = 0x00200000;
	HEAP_START = HEAP; // For malloc function

	// Initialize VESA video
	vesa_init(vid_info->info);
	u32 terminal_background[3] = { 0, 0, 0 };
	vesa_fill(terminal_background);

	// Serial connection
	serial_install();
	serial_print("\nConnected.\n");

	// Install drivers
	interrupts_install();
	timer_install();
	keyboard_install();

	// Enable drivers
	sti();

	ls_root();
	gui_init(FONT_PATH);

	gui_term_write("Wake up, " USERNAME "...\n");

	syscall_init();
	proc_init();

	idle();
}
