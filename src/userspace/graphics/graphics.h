#ifndef MELVIX_GRAPHICS_H
#define MELVIX_GRAPHICS_H

#include <stdint.h>

struct font {
	uint16_t font_32[758][32];
	uint16_t font_24[758][24];
	uint8_t font_16[758][16];
	uint16_t cursor[19];
};

struct userspace_pointers {
	unsigned char *fb;
	struct font *font;
};

void init_framebuffer();

#endif