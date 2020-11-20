/*
png -- derived from LodePNG version 20100808

Copyright (c) 2005-2010 Lode Vandevenne
Copyright (c) 2010 Sean Middleditch
Copyright (c) 2020 Marvin Borner

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:
		1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software
		in a product, an acknowledgment in the product documentation would be
		appreciated but is not required.
		2. Altered source versions must be plainly marked as such, and must not be
		misrepresented as being the original software.
		3. This notice may not be removed or altered from any source
		distribution.
*/

#ifndef PNG_H
#define PNG_H

#include <bmp.h>
#include <def.h>

enum png_error {
	PNG_EOK = 0, // Success (no error)
	PNG_ENOMEM = 1, // Memory allocation failed
	PNG_ENOTFOUND = 2, // Resource not found (file missing)
	PNG_ENOTPNG = 3, // Image data does not have a PNG header
	PNG_EMALFORMED = 4, // Image data is not a valid PNG image
	PNG_EUNSUPPORTED = 5, // Critical PNG chunk type is not supported
	PNG_EUNINTERLACED = 6, // Image interlacing is not supported
	PNG_EUNFORMAT = 7, // Image color format is not supported
	PNG_EPARAM = 8 // Invalid parameter to method call
};

enum png_format {
	PNG_BADFORMAT,
	PNG_RGB8,
	PNG_RGB16,
	PNG_RGBA8,
	PNG_RGBA16,
	PNG_LUMINANCE1,
	PNG_LUMINANCE2,
	PNG_LUMINANCE4,
	PNG_LUMINANCE8,
	PNG_LUMINANCE_ALPHA1,
	PNG_LUMINANCE_ALPHA2,
	PNG_LUMINANCE_ALPHA4,
	PNG_LUMINANCE_ALPHA8
};

enum png_state { PNG_ERROR = -1, PNG_DECODED = 0, PNG_HEADER = 1, PNG_NEW = 2 };

enum png_color { PNG_LUM = 0, PNG_RGB = 2, PNG_LUMA = 4, PNG_RGBA = 6 };

struct png_source {
	const u8 *buffer;
	u32 size;
	char owning;
};

struct png {
	u32 width;
	u32 height;

	enum png_color color_type;
	u32 color_depth;
	enum png_format format;

	u8 *buffer;
	u32 size;

	enum png_error error;
	u32 error_line;

	enum png_state state;
	struct png_source source;
};

struct bmp *png_load(const char *path);
void png_free(struct png *png);
u32 png_get_bpp(const struct png *png);

#endif
