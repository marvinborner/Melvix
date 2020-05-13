#include <fs/load.h>
#include <graphics/vesa.h>
#include <lib/stdio.h>
#include <lib/stdlib.h>
#include <memory/alloc.h>
#include <memory/paging.h>
#include <system.h>

void vbe_error()
{
	log("Error in VESA detection script!");
	warn(RED "Melvix can't work without VESA Support!" RES);
	halt_loop();
}

void vbe_set_mode(u16 mode)
{
	regs16_t regs;
	regs.ax = 0x4F02;
	regs.bx = mode;
	regs.bx |= 0x4000;
	v86(0x10, &regs);

	if (regs.ax != 0x004F)
		vbe_error();
}

u16 *vbe_get_modes()
{
	char *info_address = (char *)0x7E00;
	strcpy(info_address, "VBE2");
	for (int i = 4; i < 512; i++)
		*(info_address + i) = 0;

	regs16_t regs;
	regs.ax = 0x4F00;
	regs.es = 0;
	regs.di = 0x7E00;
	v86(0x10, &regs);

	struct vbe_info *info = (struct vbe_info *)info_address;

	if (regs.ax != 0x004F || strcmp(info->signature, "VESA") != 0)
		vbe_error();

	// Get number of modes
	u16 *mode_ptr = (u16 *)info->video_modes;
	u32 number_modes = 1;
	for (u16 *p = mode_ptr; *p != 0xFFFF; p++)
		number_modes++;

	u16 *ret = (u16 *)malloc(sizeof(u16) * number_modes);
	for (u32 i = 0; i < number_modes; i++)
		ret[i] = ((u16 *)info->video_modes)[i];

	return ret;
}

struct vbe_mode_info *vbe_get_mode_info(u16 mode)
{
	regs16_t regs;
	regs.ax = 0x4F01;
	regs.cx = mode;
	regs.es = 0;
	regs.di = 0x7E00;
	v86(0x10, &regs);
	if (regs.ax != 0x004f)
		return 0;

	struct vbe_mode_info_all *mode_info = (struct vbe_mode_info_all *)0x7E00;

	struct vbe_mode_info *ret = (struct vbe_mode_info *)malloc(sizeof(struct vbe_mode_info));
	ret->attributes = mode_info->attributes;
	ret->pitch = mode_info->pitch;
	ret->width = mode_info->width;
	ret->height = mode_info->height;
	ret->bpp = mode_info->bpp;
	ret->memory_model = mode_info->memory_model;
	ret->framebuffer = mode_info->framebuffer;

	return ret;
}

void set_optimal_resolution()
{
	log("Switching to graphics mode");
	log("Trying to detect available modes");
	u16 *video_modes = vbe_get_modes();

	u16 highest = 0;

	for (u16 *mode = video_modes; *mode != 0xFFFF; mode++) {
		struct vbe_mode_info *mode_info = vbe_get_mode_info(*mode);

		if (mode_info == 0 || (mode_info->attributes & 0x90) != 0x90 ||
		    (mode_info->memory_model != 4 && mode_info->memory_model != 6)) {
			free(mode_info);
			continue;
		}

		if (mode_info->width > vbe_width ||
		    (mode_info->width == vbe_width && (mode_info->bpp >> 3) > vbe_bpl)) {
			// if (mode_info->bpp == 32) { // Force specific bpp for debugging
			debug("Found mode: %dx%dx%d", mode_info->width, mode_info->height,
			      mode_info->bpp);
			highest = *mode;
			current_mode_info = mode_info;
			vbe_width = mode_info->width;
			vbe_height = mode_info->height;
			vbe_pitch = mode_info->pitch;
			vbe_bpl = mode_info->bpp >> 3;
			fb = (u8 *)mode_info->framebuffer;
		}
		free(mode_info);
	}

	free(video_modes);

	if (highest == 0) {
		log("Mode detection failed!");
		log("Trying common modes...");
		struct vbe_mode_info *mode_info;
		int modes[] = {
			322, 287, 286, 285, 284, // 1600x1200
			356, 355, 354, 353, 352, // 1440x900
			317, 283, 282, 281, 263, 262, // 1280x1024
			361, 360, 359, 358, 357, // 1152x720
			312, 280, 279, 278, 261, 260, // 1024x768
			366, 365, 364, 363, 362, // 1024x640
			307, 306, 305, 304, 303, // 896x672
			302, 277, 267, 275, 259, 258, 106, // 800x600
			371, 370, 369, 368, 367, // 800x500
			297, 274, 273, 272, 257, // 640x480
			292, 291, 290, 289, 256, // 640x400
			271, 270, 269, // 320x200
		};

		for (u32 i = 0; i < sizeof(modes) / sizeof(modes[0]); i++) {
			mode_info = vbe_get_mode_info((u16)modes[i]);
			if (mode_info == 0 || (mode_info->attributes & 0x90) != 0x90 ||
			    (mode_info->memory_model != 4 && mode_info->memory_model != 6)) {
				free(mode_info);
				continue;
			}

			if ((mode_info->width > vbe_width ||
			     (mode_info->width == vbe_width && (mode_info->bpp >> 3) > vbe_bpl))) {
				highest = (u16)modes[i];
				vbe_width = mode_info->width;
				vbe_height = mode_info->height;
				vbe_pitch = mode_info->pitch;
				vbe_bpl = mode_info->bpp >> 3;
				fb = (u8 *)mode_info->framebuffer;
			}
			free(mode_info);
		}

		// Everything else failed :(
		if (highest == 0)
			vbe_error();
	} else {
		log("Mode detection succeeded");
	}

	vbe_set_mode(highest);

	/* u32 fb_size = vbe_width * vbe_height * vbe_bpl; */
	/* cursor_buffer = malloc(fb_size); */
	/* for (u32 z = 0; z < fb_size; z += PAGE_S) { */
	/* 	paging_map_user(paging_kernel_directory, (u32)fb + z, (u32)fb + z); */
	/* } */

	/* dev_make("fb", NULL, (write)fb_write); */
	/* struct fs_node *node = (struct fs_node *)malloc(sizeof(struct fs_node)); */
	/* strcpy(node->name, "/dev/fb"); */
	/* fs_open(node); */
	/* node->write = (write)fb_write; */
	/* node->dev->block_size = 0; */

	if (vbe_height > 1440)
		vesa_set_font(32);
	else if (vbe_height > 720)
		vesa_set_font(24);
	else
		vesa_set_font(16);
	//vesa_set_color(default_text_color);
	//vesa_clear();

	//vesa_set_color(vesa_blue);
	//vesa_set_color(default_text_color);

	info("Successfully switched to video mode!");

	log("Using mode: (0x%x) %dx%dx%d", highest, vbe_width, vbe_height, vbe_bpl << 3);
}

const u32 default_text_color = vesa_white;
const u32 default_background_color = vesa_black;
u32 terminal_color[3] = { 0xab, 0xb2, 0xbf };
u32 terminal_background[3] = { 0x1d, 0x1f, 0x24 };
u16 terminal_x = 0;
u16 terminal_y = 0;
int font_width;
int font_height;

void vesa_set_font(int height)
{
	font_width = height / 2;
	font_height = height;
}

void vesa_set_pixel(u16 x, u16 y, const u32 color[3])
{
	u8 pos = (u8)(x * vbe_bpl + y * vbe_pitch);
	char *draw = (char *)&fb[pos];
	draw[pos] = (char)color[2];
	draw[pos + 1] = (char)color[1];
	draw[pos + 2] = (char)color[0];
}

void vesa_draw_rectangle(int x1, int y1, int x2, int y2, const u32 color[3])
{
	int pos1 = x1 * vbe_bpl + y1 * vbe_pitch;
	char *draw = (char *)&fb[pos1];
	for (int i = 0; i <= y2 - y1; i++) {
		for (int j = 0; j <= x2 - x1; j++) {
			draw[vbe_bpl * j] = (char)color[2];
			draw[vbe_bpl * j + 1] = (char)color[1];
			draw[vbe_bpl * j + 2] = (char)color[0];
		}
		draw += vbe_pitch;
	}
}

void vesa_clear()
{
	log("%dx%dx%d at %x", vbe_width, vbe_height, vbe_bpl << 3, fb);
	vesa_draw_rectangle(0, 0, vbe_width - 1, vbe_height - 1, terminal_background);
	terminal_x = 0;
	terminal_y = 0;
}

void vesa_draw_char(char ch)
{
	if (ch >= ' ') {
		int pos = terminal_x * vbe_bpl + terminal_y * vbe_pitch;
		char *draw = (char *)&fb[pos];
		u16 bitmap = 0;

		for (int cy = 0; cy <= font_height; cy++) {
			if (font_height == 16)
				bitmap = font->font_16[ch - 32][cy];
			else if (font_height == 24)
				bitmap = font->font_24[ch - 32][cy] >> 4;
			else if (font_height == 32)
				bitmap = font->font_32[ch - 32][cy];
			for (int cx = 0; cx <= font_width + 1; cx++) {
				if (bitmap &
				    ((1 << font_width) >> cx)) { // Side effect: Smoothness factor!
					draw[vbe_bpl * cx] = (char)terminal_color[2];
					draw[vbe_bpl * cx + 1] = (char)terminal_color[1];
					draw[vbe_bpl * cx + 2] = (char)terminal_color[0];
				} else {
					draw[vbe_bpl * cx] = (char)terminal_background[2];
					draw[vbe_bpl * cx + 1] = (char)terminal_background[1];
					draw[vbe_bpl * cx + 2] = (char)terminal_background[0];
				}
			}
			draw += vbe_pitch;
		}

		terminal_x += font_width;
	} else if (ch == '\n') {
		terminal_x = 0;
		terminal_y += font_height;
	} else if (ch == '\t') {
		terminal_x += 4 * font_width;
	}

	if (terminal_x >= vbe_width) {
		terminal_x = 0;
		terminal_y += font_height;
	}
}

int prev_coords[2] = {};
int first = 1; // TODO: Better initial cursor buffer solution
void vesa_draw_cursor(int x, int y)
{
	// Reset previous area
	char *reset = (char *)&fb[prev_coords[0] * vbe_bpl + prev_coords[1] * vbe_pitch];
	char *prev = (char *)&cursor_buffer[prev_coords[0] * vbe_bpl + prev_coords[1] * vbe_pitch];
	if (!first) {
		for (int cy = 0; cy <= 19; cy++) {
			for (int cx = 0; cx <= 12; cx++) {
				reset[vbe_bpl * cx] = prev[vbe_bpl * cx];
				reset[vbe_bpl * cx + 1] = prev[vbe_bpl * cx + 1];
				reset[vbe_bpl * cx + 2] = prev[vbe_bpl * cx + 2];
			}
			reset += vbe_pitch;
			prev += vbe_pitch;
		}
	}
	first = 0;

	// Draw cursor
	prev_coords[0] = x;
	prev_coords[1] = y;
	prev = (char *)&cursor_buffer[x * vbe_bpl + y * vbe_pitch];
	char *draw = (char *)&fb[x * vbe_bpl + y * vbe_pitch];
	for (int cy = 0; cy <= 19; cy++) {
		for (int cx = 0; cx <= 12; cx++) {
			// Performance issue?
			prev[vbe_bpl * cx] = draw[vbe_bpl * cx];
			prev[vbe_bpl * cx + 1] = draw[vbe_bpl * cx + 1];
			prev[vbe_bpl * cx + 2] = draw[vbe_bpl * cx + 2];
			if (font->cursor[cy] & ((1 << 12) >> cx)) {
				draw[vbe_bpl * cx] = (char)terminal_color[2];
				draw[vbe_bpl * cx + 1] = (char)terminal_color[1];
				draw[vbe_bpl * cx + 2] = (char)terminal_color[0];
			}
		}
		draw += vbe_pitch;
		prev += vbe_pitch;
	}
}