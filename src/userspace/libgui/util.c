#include <gui.h>
#include <stdint.h>

void gui_convert_color(u32 *color_array, u32 color)
{
	u8 red = (u8)((color >> 16) & 255);
	u8 green = (u8)((color >> 8) & 255);
	u8 blue = (u8)(color & 255);

	if ((vbe_bpl << 3) == 8) {
		u32 new_color =
			((red * 7 / 255) << 5) + ((green * 7 / 255) << 2) + (blue * 3 / 255);
		color_array[0] = (new_color >> 16) & 255;
		color_array[1] = (new_color >> 8) & 255;
		color_array[2] = new_color & 255;
	} else if ((vbe_bpl << 3) == 16) {
		u32 new_color =
			(((red & 0b11111000) << 8) + ((green & 0b11111100) << 3) + (blue >> 3));
		color_array[0] = (new_color >> 16) & 255;
		color_array[1] = (new_color >> 8) & 255;
		color_array[2] = new_color & 255;
	} else if ((vbe_bpl << 3) == 24 || (vbe_bpl << 3) == 32) {
		color_array[0] = red;
		color_array[1] = green;
		color_array[2] = blue;
	}
}

void gui_set_color(u32 color)
{
	gui_convert_color(terminal_color, color);
	gui_convert_color(terminal_background, gui_black);
}