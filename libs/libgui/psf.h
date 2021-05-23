// MIT License, Copyright (c) 2020 Marvin Borner
// PSF parser

#ifndef PSF_H
#define PSF_H

#include <def.h>

/**
 * PSF version 1
 */

#define PSF1_MAGIC_0 0x36
#define PSF1_MAGIC_1 0x04
#define PSF1_MODE_256 0
#define PSF1_MODE_512 1
#define PSF1_MODE_256_UNICODE 2
#define PSF1_MODE_512_UNICODE 3

struct psf1_header {
	u8 magic[2];
	u8 mode;
	u8 char_size;
};

/**
 * PSF version 2
 */

#define PSF2_MAGIC_0 0x72
#define PSF2_MAGIC_1 0xb5
#define PSF2_MAGIC_2 0x4a
#define PSF2_MAGIC_3 0x86

struct psf2_header {
	u8 magic[4];
	u32 version;
	u32 size;
	u32 flags;
	u32 glyph_count;
	u32 char_size;
	u32 height;
	u32 width;
};

struct gfx_font *psf_parse(char *data) NONNULL;

#endif
