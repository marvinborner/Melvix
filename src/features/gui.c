// MIT License, Copyright (c) 2020 Marvin Borner
// Some GUI functions

#include <fs.h>
#include <gui.h>
#include <psf.h>
#include <str.h>
#include <vesa.h>

struct font *font;

void gui_write_char(int x, int y, const u32 c[3], char ch)
{
	int pos = x * vbe_bpl + y * vbe_pitch;
	char *draw = (char *)&fb[pos];

	u32 stride = font->char_size / font->height;
	for (int cy = 0; cy < font->height; cy++) {
		for (int cx = 0; cx < font->width; cx++) {
			u8 bits = font->chars[ch * font->char_size + cy * stride + cx / 8];
			u8 bit = bits >> (7 - cx % 8) & 1;
			if (bit) {
				draw[vbe_bpl * cx] = c[2];
				draw[vbe_bpl * cx + 1] = c[1];
				draw[vbe_bpl * cx + 2] = c[0];
			}
		}
		draw += vbe_pitch;
	}
}

void gui_write(int x, int y, const u32 c[3], char *text)
{
	for (u32 i = 0; i < strlen(text); i++) {
		gui_write_char(x + i * font->width, y, c, text[i]);
	}
}

// Abstraction
int x, y = 1;
const u32 c[3] = { 0xff, 0xff, 0xff };
void gui_term_write_char(char ch)
{
	if (x + font->width > vbe_width) {
		x = 0;
		y += font->height;
	}

	if (ch >= ' ' && ch <= '~') {
		gui_write_char(x, y, c, ch);
		x += font->width;
	} else if (ch == '\n') {
		x = 0;
		y += font->height;
	}
}

void gui_term_write(char *text)
{
	for (u32 i = 0; i < strlen(text); i++) {
		gui_term_write_char(text[i]);
	}
}

void gui_init(char *font_path)
{
	font = psf_parse(read_file(font_path));
}
