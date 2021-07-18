// MIT License, Copyright (c) 2021 Marvin Borner

#include <drivers/vga.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_ADDRESS 0xb8000

void vga_clear(void)
{
	u16 *out = (u16 *)VGA_ADDRESS;
	for (u16 i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
		out[i] = 0;
}

void vga_put_at(char ch, u8 x, u8 y, u8 color)
{
	u8 *out = (u8 *)(VGA_ADDRESS + 2 * (x + y * VGA_WIDTH));
	*out++ = ch;
	*out++ = color;
}

void vga_print(const char *data)
{
	static u8 x = 0, y = 0;

	for (const char *p = data; *p; p++) {
		if (*p == '\n') {
			x = 0;
			y++;
			continue;
		} else if (x + 1 == VGA_WIDTH) {
			x = 0;
			y++;
		} else if (y + 1 == VGA_HEIGHT) {
			x = 0;
			y = 0;
			vga_clear();
		}

		vga_put_at(*p, x++, y, 0x07);
	}
}
