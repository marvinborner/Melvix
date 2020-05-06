#ifndef MELVIX_VESA_H
#define MELVIX_VESA_H

#include <stdint.h>
#include <system.h>

/**
 * The CPUs response to the 0x4F00 call
 * Used to receive the supported video modes
 */
struct vbe_info {
	char signature[4];
	u16 version;
	u32 oem;
	u32 capabilities;
	u32 video_modes;
	u16 video_memory;
	u16 software_rev;
	u32 vendor;
	u32 product_name;
	u32 product_rev;
	char reserved[222];
	char oem_data[256];
} __attribute__((packed));

/**
 * The CPUs response to the 0x4F01 call
 * Used to get information about a specific video mode code
 */
struct vbe_mode_info_all {
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

	u32 framebuffer;
	u32 off_screen_mem_off;
	u16 off_screen_mem_size;
	u8 reserved1[206];
} __attribute__((packed));

struct vbe_mode_info {
	u16 attributes;
	u16 pitch;
	u16 width;
	u16 height;
	u8 bpp;
	u8 memory_model;
	u32 framebuffer;
} __attribute__((packed));

/**
 * Set the video mode to a specified resolution using
 * a video mode code
 * @param mode The requested video mode code from 0x4F00 call
 */
void vbe_set_mode(unsigned short mode);

/**
 * Find the highest resolution using 0x4F00 and call
 * vbe_set_mode using the video_modes far_ptr
 */
void set_optimal_resolution();

/**
 * Sets one of the fonts inside the font header file
 * @param height The desired font height
 */
void vesa_set_font(int height);

/**
 * Draws a single char
 * @param ch The char
 */
void vesa_draw_char(char ch);

/**
 * Draw a char from keyboard
 * @param ch The character
 */
void vesa_keyboard_char(char ch);

/**
 * Draw a string in VESA mode
 * @param data The string
 */
void vesa_draw_string(const char *data);

/**
 * Draw a number in VESA mode
 * @param n The number
 */
void vesa_draw_number(int n);

/**
 * Updates the cursor
 * @param x The X position
 * @param y The Y position
 */
void vesa_draw_cursor(int x, int y);

/**
 * Sets the color using a rgb number
 * @param color The color
 */
void vesa_set_color(u32 color);

/**
 * An enum with vesa colors
 * From https://github.com/joshdick/onedark.vim/ License: MIT
 */
enum vesa_color {
	vesa_black = 0x1d1f24,
	vesa_red = 0xE06C75,
	vesa_green = 0x98C379,
	vesa_yellow = 0xE5C07B,
	vesa_blue = 0x61AFEF,
	vesa_magenta = 0xC678DD,
	vesa_cyan = 0x56B6C2,
	vesa_white = 0xABB2BF,
	vesa_dark_black = 0x3E4452,
	vesa_dark_red = 0xBE5046,
	vesa_dark_green = 0x98C379,
	vesa_dark_yellow = 0xD19A66,
	vesa_dark_blue = 0x61AFEF,
	vesa_dark_magenta = 0xC678DD,
	vesa_dark_cyan = 0x56B6C2,
	vesa_dark_white = 0x5C6370,
};

/**
 * The default text color
 */
const u32 default_text_color;

/**
 * The current text color (as normalized array)
 */
u32 terminal_color[3];

/**
 * The current input
 */
char text[1024];

/**
 * The current video mode
 */
struct vbe_mode_info *current_mode_info;

/**
 * The width of the current video mode
 */
int vbe_width;

/**
 * The height of the current video mode
 */
int vbe_height;

/**
 * The pitch (bytes per line) of the current video mode
 */
int vbe_pitch;

/**
 * The bytes per line (pixel width) of the current video mode
 */
int vbe_bpl;

/**
 * The framebuffer interface
 */
u8 *fb;

u8 *cursor_buffer;

#endif