// MIT License, Copyright (c) 2020 Marvin Borner
// Some GUI functions

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

void gui_init(char *font_path)
{
	font = psf_parse(read(font_path));
}
