// MIT License, Copyright (c) 2020 Marvin Borner
// PSF parser

#include <def.h>
#include <print.h>
#include <psf.h>
#include <vesa.h>

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

// Will be removed in the future
void psf_test(char *chars, int height, int width, int char_size)
{
	char ch = 'a';
	int x = 50;
	int y = 400;
	const u32 c[3] = { 0xff, 0x00, 0x00 };

	printf("%d %d %d\n", height, width, char_size);

	int pos = x * vbe_bpl + y * vbe_pitch;
	char *draw = (char *)&fb[pos];
	u16 row = 0;

	for (int cy = 0; cy <= height; cy++) {
		row = chars[ch * char_size + cy * ((width + 7) / 8)];

		for (int cx = 0; cx <= width + 1; cx++) {
			if (row & (1 << (width - 1))) {
				draw[vbe_bpl * cx] = (char)c[2];
				draw[vbe_bpl * cx + 1] = (char)c[1];
				draw[vbe_bpl * cx + 2] = (char)c[0];
			}
			/* } else { */
			/* 	draw[vbe_bpl * cx] = (char)c[2]; */
			/* 	draw[vbe_bpl * cx + 1] = (char)terminal_background[1]; */
			/* 	draw[vbe_bpl * cx + 2] = (char)terminal_background[0]; */
			/* } */
			row <<= 1;
		}
		draw += vbe_pitch;
	}
}

char *psf_parse(char *data)
{
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
		return 0;
	}

	psf_test(chars, height, width, char_size);

	return chars;
}
