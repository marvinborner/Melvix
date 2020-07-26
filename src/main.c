// MIT License, Copyright (c) 2020 Marvin Borner

#include "config.h"
#include <boot.h>
#include <def.h>
#include <fs.h>
#include <gui.h>
#include <interrupts.h>
#include <keyboard.h>
#include <print.h>
#include <serial.h>

u32 HEAP = 0x00200000;
u32 HEAP_START;

void main(struct mem_info *mem_info, struct vid_info *vid_info)
{
	HEAP_START = HEAP; // For malloc function
	interrupts_install();
	keyboard_install();

	mem_info++; // TODO: Use the mmap (or remove)!

	vesa_init(vid_info->info);
	u32 terminal_background[3] = { 0, 0, 0 };
	vesa_fill(terminal_background);

	serial_install();

	ls_root();
	gui_init(FONT_PATH);

	gui_term_write("Welcome back, " USERNAME ".\n");

	while (1) {
	};
}
