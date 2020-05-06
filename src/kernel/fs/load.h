#ifndef MELVIX_LOAD_H
#define MELVIX_LOAD_H

#include <stdint.h>

struct font *font;

struct font {
	u16 font_32[758][32];
	u16 font_24[758][24];
	u8 font_16[758][16];
	u16 cursor[19];
};

void load_binaries();

#endif