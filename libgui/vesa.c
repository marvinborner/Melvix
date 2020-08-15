// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <vesa.h>

void vesa_draw_rectangle(struct vbe *vbe, int x1, int y1, int x2, int y2, const u32 color[3])
{
	int bpl = vbe->bpp >> 3;

	int pos1 = x1 * bpl + y1 * vbe->pitch;
	u8 *draw = &vbe->fb[pos1];
	for (int i = 0; i <= y2 - y1; i++) {
		for (int j = 0; j <= x2 - x1; j++) {
			draw[bpl * j] = color[2];
			draw[bpl * j + 1] = color[1];
			draw[bpl * j + 2] = color[0];
		}
		draw += vbe->pitch;
	}
}

void vesa_set_pixel(struct vbe *vbe, u16 x, u16 y, const u32 color[3])
{
	u8 pos = x * (vbe->bpp >> 3) + y * vbe->pitch;
	u8 *draw = &vbe->fb[pos];
	draw[pos] = (char)color[2];
	draw[pos + 1] = (char)color[1];
	draw[pos + 2] = (char)color[0];
}

void vesa_fill(struct vbe *vbe, const u32 color[3])
{
	vesa_draw_rectangle(vbe, 0, 0, vbe->width - 1, vbe->height - 1, color);
}
