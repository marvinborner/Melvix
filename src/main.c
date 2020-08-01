// MIT License, Copyright (c) 2020 Marvin Borner

#include "config.h"
#include <boot.h>
#include <def.h>
#include <fs.h>
#include <gui.h>
#include <interrupts.h>
#include <keyboard.h>
#include <load.h>
#include <print.h>
#include <serial.h>
#include <timer.h>

u32 HEAP = 0x00200000;
u32 HEAP_START;

void kernel_main(struct mem_info *mem_info, struct vid_info *vid_info)
{
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
	__asm__("sti");

	mem_info++; // TODO: Use the mmap (or remove)!

	ls_root();
	gui_init(FONT_PATH);

	gui_term_write("Wake up, " USERNAME "...\n");
	bin_load("/test");

	while (1) {
	};
}
