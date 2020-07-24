// MIT License, Copyright (c) 2020 Marvin Borner

#include <boot.h>
#include <def.h>
#include <fs.h>
#include <interrupts.h>
#include <keyboard.h>
#include <print.h>
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
	vbe = vid_info->info;

	u8 terminal_background[3] = { 0x1d, 0x1f, 0x24 };
	vesa_fill(terminal_background);

	serial_install();
	printf("hello\n");

	ls_root();
	printf("%s", read_file("test.txt", 2));

	while (1) {
	};
}