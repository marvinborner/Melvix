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

// Abstraction
int x, y = 0;
const u32 c[3] = { 0xff, 0xff, 0xff };
void gui_term_write_char(struct vbe *vbe, char ch)
{
	if (x + font->width > vbe->width) {
		x = 0;
		y += font->height;
	}

	if (ch >= ' ' && ch <= '~') {
		gui_write_char(vbe, x, y, c, ch);
		x += font->width;
	} else if (ch == '\n') {
		x = 0;
		y += font->height;
	}
}

void gui_term_write(struct vbe *vbe, char *text)
{
	for (u32 i = 0; i < strlen(text); i++) {
		gui_term_write_char(vbe, text[i]);
	}
}

void gui_init(char *font_path)
{
	font = psf_parse(read(font_path));
}
