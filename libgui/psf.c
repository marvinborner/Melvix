// MIT License, Copyright (c) 2020 Marvin Borner
// PSF parser

#include <def.h>
#include <gfx.h>
#include <mem.h>
#include <print.h>
#include <psf.h>

// Verifies the PSF magics
// Returns the PSF version or 0
int psf_verify(char *data)
{
	struct psf1_header *header1 = (struct psf1_header *)data;
	struct psf2_header *header2 = (struct psf2_header *)data;

	if (header1->magic[0] == PSF1_MAGIC_0 && header1->magic[1] == PSF1_MAGIC_1)
		return 1;
	else if (header2->magic[0] == PSF2_MAGIC_0 && header2->magic[1] == PSF2_MAGIC_1 &&
		 header2->magic[2] == PSF2_MAGIC_2 && header2->magic[3] == PSF2_MAGIC_3)
		return 2;
	else
		return 0;
}

struct font *psf_parse(char *data)
{
	if (!data)
		return NULL;

	int version = psf_verify(data);

	char *chars;
	int height;
	int width;
	int char_size;

	if (version == 1) {
		chars = data + sizeof(struct psf1_header);
		height = ((struct psf1_header *)data)->char_size;
		width = 8;
		char_size = ((struct psf1_header *)data)->char_size;
	} else if (version == 2) {
		chars = data + ((struct psf2_header *)data)->size;
		height = ((struct psf2_header *)data)->height;
		width = ((struct psf2_header *)data)->width;
		char_size = ((struct psf2_header *)data)->char_size;
	} else {
		print("Unknown font!\n");
		return NULL;
	}

	struct font *font = malloc(sizeof(*font));
	font->chars = chars;
	font->size.x = width;
	font->size.y = height;
	font->char_size = char_size;

	return font;
}
