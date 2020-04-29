#include <stdint.h>
#include <gui.h>

void gui_draw_rectangle(int x1, int y1, int x2, int y2, const u32 color[3])
{
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

void gui_screen_clear()
{
	gui_draw_rectangle(0, 0, vbe_width - 1, vbe_height - 1, terminal_background);
}