#include <def.h>
#include <vesa.h>

void vesa_draw_rectangle(int x1, int y1, int x2, int y2, const u32 color[3])
{
	int vbe_bpl = vbe->bpp >> 3;
	int vbe_pitch = vbe->pitch;
	u8 *fb = (u8 *)vbe->framebuffer;

	int pos1 = x1 * vbe_bpl + y1 * vbe_pitch;
	char *draw = (char *)&fb[pos1];
	for (int i = 0; i <= y2 - y1; i++) {
		for (int j = 0; j <= x2 - x1; j++) {
			draw[vbe_bpl * j] = (char)color[2];
			draw[vbe_bpl * j + 1] = (char)color[1];
			draw[vbe_bpl * j + 2] = (char)color[0];
		}
		draw += vbe_pitch;
	}
}

void vesa_clear(const u32 color[3])
{
	vesa_draw_rectangle(0, 0, vbe->width - 1, vbe->height - 1, color);
}
