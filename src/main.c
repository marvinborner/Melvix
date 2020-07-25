// MIT License, Copyright (c) 2020 Marvin Borner

#include <boot.h>
#include <def.h>
#include <fs.h>
#include <interrupts.h>
#include <keyboard.h>
#include <print.h>
#include <psf.h>
#include <serial.h>
#include <vesa.h>

u32 HEAP = 0x00200000;
u32 HEAP_START;

void main(struct mem_info *mem_info, struct vid_info *vid_info)
{
	HEAP_START = HEAP; // For malloc function
	interrupts_install();
	keyboard_install();

	mem_info++; // TODO: Use the mmap (or remove)!

	vesa_init(vid_info->info);
	u32 terminal_background[3] = { 0x1d, 0x1f, 0x24 };
	vesa_fill(terminal_background);

	serial_install();

	ls_root();
	/* psf_parse(read_file("/font/spleen-8x16.psfu")); */
	psf_parse(read_file("/font/spleen-16x32.psfu"));

	while (1) {
	};
}
