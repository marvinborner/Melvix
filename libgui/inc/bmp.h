// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef BMP_H
#define BMP_H

#include <def.h>
#include <vec.h>

struct bmp_header {
	u8 signature[2];
	u32 size;
	u32 reserved;
	u32 offset;
} __attribute__((packed));

struct bmp_info {
	u32 size;
	u32 width;
	u32 height;
	u16 planes;
	u16 bpp;
	u32 compression;
	u32 compressed_size;
	u32 x_pixel_meter;
	u32 y_pixel_meter;
	u32 colors;
	u32 important_colors;
};

struct bmp {
	vec2 size;
	u8 *data;
	u32 bpp;
	u32 pitch;
};

struct bmp *bmp_load(const char *path);

#endif
