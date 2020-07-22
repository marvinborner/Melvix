// MIT License, Copyright (c) 2020 Marvin Borner

#include <boot.h>
#include <def.h>
#include <serial.h>
#include <vesa.h>

u32 HEAP = 0x00200000;
u32 HEAP_START;

void main(struct mem_info *mem_info, struct vid_info *vid_info)
{
	HEAP_START = HEAP; // For malloc function

	mem_info++; // TODO: Use the mmap (or remove)!
	vbe = vid_info->info;

	u8 terminal_background[3] = { 0x1d, 0x1f, 0x24 };
	vesa_fill(terminal_background);

	serial_install();
	serial_print("hello\n");

	while (1) {
	};
}
