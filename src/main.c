// MIT License, Copyright (c) 2020 Marvin Borner

#include <boot.h>
#include <def.h>
#include <serial.h>
#include <vesa.h>

void main(struct mem_info *mem_info, struct vid_info *vid_info)
{
	mem_info++; // TODO: Use the mmap (or remove)!
	vbe = vid_info->info;

	u32 terminal_background[3] = { 0x1d, 0x1f, 0x24 };
	vesa_clear(terminal_background);

	serial_install();
	serial_print("hello\n");

	while (1) {
	};
}
