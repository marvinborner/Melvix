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

void gui_load_image(struct window *win, char *path, int x, int y)
{
	struct bmp *bmp = bmp_load(path);
	assert(bmp && bmp->width + x <= win->width);
	assert(bmp && bmp->height + y <= win->height);

	// TODO: Support padding with odd widths
	u8 *srcfb = bmp->data;
	u8 *destfb = win->fb;
	int bpl = bmp->bpp >> 3;
	for (u32 cy = 0; cy <= bmp->height; cy++) {
		for (u32 cx = 0; cx <= bmp->width; cx++) {
			destfb[bpl * cx + 0] = srcfb[bpl * cx + 0];
			destfb[bpl * cx + 1] = srcfb[bpl * cx + 1];
			destfb[bpl * cx + 2] = srcfb[bpl * cx + 2];
		}
		srcfb += bmp->pitch;
		destfb += win->pitch;
	}
}

void gui_load_wallpaper(struct window *win, char *path)
{
	gui_load_image(win, path, 0, 0);
}

void gui_win_on_win(struct window *src, struct window *dest, int x, int y)
{
	// TODO: x, y image coords
	(void)x;
	(void)y;

	u8 *srcfb = src->fb;
	u8 *destfb = dest->fb;
	int bpl = dest->bpp >> 3;
	for (u32 cy = 0; cy <= src->height; cy++) {
		for (u32 cx = 0; cx <= src->width; cx++) {
			destfb[bpl * cx + 0] = srcfb[bpl * cx + 0];
			destfb[bpl * cx + 1] = srcfb[bpl * cx + 1];
			destfb[bpl * cx + 2] = srcfb[bpl * cx + 2];
		}
		srcfb += src->pitch;
		destfb += dest->pitch;
	}
}

void gui_draw_rectangle(struct window *win, int x1, int y1, int x2, int y2, const u32 color[3])
{
	int bpl = win->bpp >> 3;

	int pos1 = x1 * bpl + y1 * win->pitch;
	u8 *draw = &win->fb[pos1];
	for (int i = 0; i <= y2 - y1; i++) {
		for (int j = 0; j <= x2 - x1; j++) {
			draw[bpl * j + 0] = color[2];
			draw[bpl * j + 1] = color[1];
			draw[bpl * j + 2] = color[0];
		}
		draw += win->pitch;
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
