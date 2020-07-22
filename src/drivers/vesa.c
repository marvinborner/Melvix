#include <def.h>
#include <vesa.h>

void vesa_draw_rectangle(int x1, int y1, int x2, int y2, const u8 color[3])
{
	int vbe_bpl = vbe->bpp >> 3;
	int vbe_pitch = vbe->pitch;
	u8 *fb = (u8 *)vbe->framebuffer;

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

void vesa_fill(const u8 color[3])
{
	vesa_draw_rectangle(0, 0, vbe->width - 1, vbe->height - 1, color);
}
