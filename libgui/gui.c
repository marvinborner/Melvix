// MIT License, Copyright (c) 2020 Marvin Borner
// Some GUI functions

#include <assert.h>
#include <bmp.h>
#include <gui.h>
#include <psf.h>
#include <str.h>
#include <sys.h>
#include <vesa.h>

struct font *font;

void gui_write_char(struct vbe *vbe, int x, int y, const u32 c[3], char ch)
{
	int bpl = vbe->bpp >> 3;

	int pos = x * bpl + y * vbe->pitch;
	char *draw = (char *)&vbe->fb[pos];

	u32 stride = font->char_size / font->height;
	for (int cy = 0; cy < font->height; cy++) {
		for (int cx = 0; cx < font->width; cx++) {
			u8 bits = font->chars[ch * font->char_size + cy * stride + cx / 8];
			u8 bit = bits >> (7 - cx % 8) & 1;
			if (bit) {
				draw[bpl * cx] = c[2];
				draw[bpl * cx + 1] = c[1];
				draw[bpl * cx + 2] = c[0];
			}
		}
		draw += vbe->pitch;
	}
}

void gui_write(struct vbe *vbe, int x, int y, const u32 c[3], char *text)
{
	for (u32 i = 0; i < strlen(text); i++) {
		gui_write_char(vbe, x + i * font->width, y, c, text[i]);
	}
}

void gui_load_wallpaper(struct vbe *vbe, char *path)
{
	struct bmp *bmp = bmp_load(path);
	assert(bmp);

	int bpl = bmp->bpp >> 3;
	int x1 = 0;
	int x2 = bmp->width;
	int y1 = 0;
	int y2 = bmp->height;

	// TODO: Support padding with odd widths
	/* int pitch = bmp->width * bpl; */
	/* int padding = 1; */
	/* while ((pitch + padding) % 4 != 0) */
	/* 	padding++; */
	/* int psw = pitch + padding; */

	int pos1 = x1 * bpl + y1 * vbe->pitch;
	u8 *draw = &vbe->fb[pos1];
	u8 *data = bmp->data;
	for (int i = 0; i <= y2 - y1; i++) {
		for (int j = 0; j <= x2 - x1; j++) {
			draw[bpl * j] = data[bpl * j];
			draw[bpl * j + 1] = data[bpl * j + 1];
			draw[bpl * j + 2] = data[bpl * j + 2];
		}
		draw += vbe->pitch;
		data += bmp->pitch;
	}
}

void gui_draw_rectangle(struct window *win, int x1, int y1, int x2, int y2, const u32 color[3])
{
	int bpl = win->vbe->bpp >> 3;

	int pos1 = x1 * bpl + y1 * win->vbe->pitch;
	u8 *draw = &win->fb[pos1];
	for (int i = 0; i <= y2 - y1; i++) {
		for (int j = 0; j <= x2 - x1; j++) {
			draw[bpl * j] = color[2];
			draw[bpl * j + 1] = color[1];
			draw[bpl * j + 2] = color[0];
		}
		draw += win->vbe->pitch;
	}
}

void gui_fill(struct window *win, const u32 color[3])
{
	gui_draw_rectangle(win, 0, 0, win->width - 1, win->height - 1, color);
}

void gui_init(char *font_path)
{
	font = psf_parse(read(font_path));
}
