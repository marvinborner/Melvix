#ifndef MELVIX_GUI_H
#define MELVIX_GUI_H

#include <stdint.h>

struct font {
	u16 font_32[758][32];
	u16 font_24[758][24];
	u8 font_16[758][16];
	u16 cursor[19];
};

struct pointers {
	u8 *fb;
	struct font *font;
};

struct pointers *pointers;

void gui_init();

#endif