#ifndef MELVIX_GUI_H
#define MELVIX_GUI_H

#include <stdint.h>

struct vbe_mode_info {
	u16 attributes;
	u16 pitch;
	u16 width;
	u16 height;
	u8 bpp;
	u8 memory_model;
	u32 framebuffer;
};

struct font {
	u16 font_32[758][32];
	u16 font_24[758][24];
	u8 font_16[758][16];
	u16 cursor[19];
};

struct pointers {
	struct vbe_mode_info *mode_info;
	struct font *font;
};

u32 terminal_color[3];
u32 terminal_background[3];
enum gui_color {
	gui_black = 0x1d1f24,
	gui_red = 0xE06C75,
	gui_green = 0x98C379,
	gui_yellow = 0xE5C07B,
	gui_blue = 0x61AFEF,
	gui_magenta = 0xC678DD,
	gui_cyan = 0x56B6C2,
	gui_white = 0xABB2BF,
	gui_dark_black = 0x3E4452,
	gui_dark_red = 0xBE5046,
	gui_dark_green = 0x98C379,
	gui_dark_yellow = 0xD19A66,
	gui_dark_blue = 0x61AFEF,
	gui_dark_magenta = 0xC678DD,
	gui_dark_cyan = 0x56B6C2,
	gui_dark_white = 0x5C6370,
};

u8 *fb;
int vbe_width;
int vbe_height;
int vbe_pitch;
int vbe_bpl;

struct pointers *pointers;

void gui_init();

void gui_draw_rectangle(int x1, int y1, int x2, int y2, const u32 color[3]);
void gui_screen_clear();

void gui_convert_color(u32 *color_array, u32 color);

#endif