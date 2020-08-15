// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef VBE_H
#define VBE_H

#include <def.h>

struct vbe {
	u16 attributes;
	u8 window_a;
	u8 window_b;
	u16 granularity;
	u16 window_size;
	u16 segment_a;
	u16 segment_b;
	u32 win_func_ptr;
	u16 pitch;
	u16 width;
	u16 height;
	u8 w_char;
	u8 y_char;
	u8 planes;
	u8 bpp;
	u8 banks;
	u8 memory_model;
	u8 bank_size;
	u8 image_pages;
	u8 reserved0;

	u8 red_mask;
	u8 red_position;
	u8 green_mask;
	u8 green_position;
	u8 blue_mask;
	u8 blue_position;
	u8 reserved_mask;
	u8 reserved_position;
	u8 direct_color_attributes;

	u8 *fb;
	u32 off_screen_mem_off;
	u16 off_screen_mem_size;
	u8 reserved1[206];
};

void vesa_draw_rectangle(struct vbe *vbe, int x1, int y1, int x2, int y2, const u32 color[3]);
void vesa_fill(struct vbe *vbe, const u32 color[3]);
void vesa_set_pixel(struct vbe *vbe, u16 x, u16 y, const u32 color[3]);

#endif
