// MIT License, Copyright (c) 2020 Marvin Borner
// PSF parser

#include <assert.h>
#include <def.h>
#include <libgui/gfx.h>
#include <libgui/psf.h>
#include <mem.h>
#include <print.h>

static u8 psf_verify(void *data)
{
	struct psf1_header *header1 = (struct psf1_header *)data;
	struct psf2_header *header2 = (struct psf2_header *)data;

	if (header1->magic == PSF1_MAGIC)
		return 1;
	else if (header2->magic == PSF2_MAGIC)
		return 2;
	else
		return 0;
}

void psf_free(struct gfx_font *font)
{
	free(font->raw);
	free(font);
}

// TODO: Improve bruteforce unitab search
u32 psf_unicode(struct gfx_font *font, u32 needle)
{
	if (!font->flags.unicode)
		return 0;

	u8 *data = font->raw;

	u8 version = psf_verify(data);
	assert(version == 2);

	struct psf2_header *header = (struct psf2_header *)data;

	u8 *table = (u8 *)(data + header->size + header->char_count * header->char_size);

	u32 glyph = 0;
	while (table < data + font->total_size) {
		u32 ch = table[0] & 0xff;
		if (ch == 0xff) {
			glyph++;
			table++;
			continue;
		} else if (ch & 128) {
			if ((ch & 32) == 0) {
				ch = ((table[0] & 0x1f) << 6) + (table[1] & 0x3f);
				table++;
			} else if ((ch & 16) == 0) {
				ch = ((((table[0] & 0xf) << 6) + (table[1] & 0x3f)) << 6) +
				     (table[2] & 0x3f);
				table += 2;
			} else if ((ch & 8) == 0) {
				ch = ((((((table[0] & 0x7) << 6) + (table[1] & 0x3f)) << 6) +
				       (table[2] & 0x3f))
				      << 6) +
				     (table[3] & 0x3f);
				table += 3;
			} else {
				ch = 0;
			}
		}

		if (ch == needle)
			return glyph;

		table++;
	}

	return 0;
}

struct gfx_font *psf_parse(const char *path)
{
	struct stat s = { 0 };
	if (stat(path, &s) != 0 || !s.size)
		return NULL;
	u8 *data = malloc(s.size);
	read(path, data, 0, s.size);

	u8 version = psf_verify(data);
	if (version == 0) {
		print("Unknown font!\n");
		return NULL;
	} else if (version == 1) {
		log("PSF version 1 is no longer supported\n");
		return NULL;
	}

	struct psf2_header *header = (struct psf2_header *)data;
	u8 *chars = data + header->size;
	u32 height = header->height;
	u32 width = header->width;
	u32 char_size = header->char_size;

	if (!(header->flags & PSF2_UNICODE))
		log("No unicode table found, fonts may not render correctly!\n");

	struct gfx_font *font = malloc(sizeof(*font));
	font->raw = data;
	font->chars = chars;
	font->size.x = width;
	font->size.y = height;
	font->char_size = char_size;
	font->total_size = s.size;
	font->flags.unicode = (header->flags & PSF2_UNICODE) == PSF2_UNICODE;

	return font;
}
