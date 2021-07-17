// MIT License, Copyright(c) 2021 Marvin Borner

#include <drivers/vga.h>
#include <kernel.h>

void kernel_main(struct boot_information *data)
{
	UNUSED(data);
	vga_clear();
	vga_print("OK!\n");
	while (1)
		;
}
