#ifndef MELVIX_VESA_H
#define MELVIX_VESA_H

#include <stdint.h>
#include <kernel/system.h>

/**
 * The CPUs response to the 0x4F00 call
 * Used to receive the supported video modes
 */
struct vbe_info {
	char signature[4];
	uint16_t version;
	uint32_t oem;
	uint32_t capabilities;
	uint32_t video_modes;
	uint16_t video_memory;
	uint16_t software_rev;
	uint32_t vendor;
	uint32_t product_name;
	uint32_t product_rev;
	char reserved[222];
	char oem_data[256];
} __attribute__((packed));

/**
 * The CPUs response to the 0x4F01 call
 * Used to get information about a specific video mode code
 */
struct vbe_mode_info_all {
	uint16_t attributes;
	uint8_t window_a;
	uint8_t window_b;
	uint16_t granularity;
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;
	uint16_t pitch;
	uint16_t width;
	uint16_t height;
	uint8_t w_char;
	uint8_t y_char;
	uint8_t planes;
	uint8_t bpp;
	uint8_t banks;
	uint8_t memory_model;
	uint8_t bank_size;
	uint8_t image_pages;
	uint8_t reserved0;

	uint8_t red_mask;
	uint8_t red_position;
	uint8_t green_mask;
	uint8_t green_position;
	uint8_t blue_mask;
	uint8_t blue_position;
	uint8_t reserved_mask;
	uint8_t reserved_position;
	uint8_t direct_color_attributes;

	uint32_t framebuffer;
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;
	uint8_t reserved1[206];
} __attribute__((packed));

struct vbe_mode_info {
	uint16_t attributes;
	uint16_t pitch;
	uint16_t width;
	uint16_t height;
	uint8_t bpp;
	uint8_t memory_model;
	uint32_t framebuffer;
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
void vesa_set_color(uint32_t color);

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
const uint32_t default_text_color;

/**
 * The current text color (as normalized array)
 */
uint32_t terminal_color[3];

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
uint8_t *fb;

uint8_t *cursor_buffer;

#endif