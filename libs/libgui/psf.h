// MIT License, Copyright (c) 2020 Marvin Borner
// PSF parser

#ifndef PSF_H
#define PSF_H

#include <def.h>

/**
 * PSF version 1
 */

#define PSF1_MAGIC 0x0436

struct psf1_header {
	u16 magic;
	u8 mode;
	u8 char_size;
};

/**
 * PSF version 2
 */

enum psf2_flags { PSF2_UNICODE = 1 };
#define PSF2_MAGIC 0x864ab572

struct psf2_header {
	u32 magic;
	u32 version;
	u32 size;
	enum psf2_flags flags;
	u32 char_count;
	u32 char_size;
	u32 height;
	u32 width;
};

struct gfx_font *psf_parse(const char *path) NONNULL;
u32 psf_unicode(struct gfx_font *font, u32 needle);
void psf_free(struct gfx_font *font);

#endif
