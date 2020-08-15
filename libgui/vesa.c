// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <vesa.h>

void vesa_draw_rectangle(int x1, int y1, int x2, int y2, const u32 color[3])
{
	int pos1 = x1 * vbe_bpl + y1 * vbe_pitch;
	u8 *draw = &fb[pos1];
	for (int i = 0; i <= y2 - y1; i++) {
		for (int j = 0; j <= x2 - x1; j++) {
			draw[vbe_bpl * j] = color[2];
			draw[vbe_bpl * j + 1] = color[1];
			draw[vbe_bpl * j + 2] = color[0];
		}
		draw += vbe_pitch;
	}
}

void vesa_set_pixel(u16 x, u16 y, const u32 color[3])
{
	u8 pos = x * vbe_bpl + y * vbe_pitch;
	u8 *draw = &fb[pos];
	draw[pos] = (char)color[2];
	draw[pos + 1] = (char)color[1];
	draw[pos + 2] = (char)color[0];
}

void vesa_fill(const u32 color[3])
{
	vesa_draw_rectangle(0, 0, vbe->width - 1, vbe->height - 1, color);
}

void vesa_init(struct vbe *info)
{
	vbe = info;
	vbe_height = vbe->height;
	vbe_width = vbe->width;
	vbe_bpl = vbe->bpp >> 3;
	vbe_pitch = vbe->pitch;
	fb = (u8 *)vbe->framebuffer;
}
