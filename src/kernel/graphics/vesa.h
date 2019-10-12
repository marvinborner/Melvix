#ifndef MELVIX_VESA_H
#define MELVIX_VESA_H

#include <stdint.h>
#include "../system.h"

int vesa_available;

struct edid_data {
    uint8_t padding[8];
    uint16_t manufacture_id;
    uint16_t product_code;
    uint32_t serial_number;
    uint8_t manufacture_week;
    uint8_t manufacture_year;
    uint8_t edid_version;
    uint8_t edid_revision;
    uint8_t video_input_type;
    uint8_t max_horizontal_size;
    uint8_t max_vertical_size;
    uint8_t gamma_factor;
    uint8_t dpms_flags;
    uint8_t chroma_information[10];
    uint8_t timings_1;
    uint8_t timings_2;
    uint8_t reserved_timings;
    uint32_t timing_identification[8];
    uint8_t timing_description_1[18];
    uint8_t timing_description_2[18];
    uint8_t timing_description_3[18];
    uint8_t timing_description_4[18];
    uint8_t unused;
    uint8_t checksum;
};

/**
 * The CPUs response to the 0x4F00 call
 * Used to receive the supported video modes
 */
struct vbe_info {
    char signature[4];
    uint32_t version;
    struct far_ptr oem;
    uint32_t capabilities;
    struct far_ptr video_modes;
    uint32_t video_memory;
    uint32_t software_rev;
    uint32_t vendor;
    uint32_t product_name;
    uint32_t product_rev;
    char reserved[222];
    char oem_data[256];
} __attribute__ ((packed));

/**
 * The CPUs response to the 0x4F01 call
 * Used to get information about a specific video mode code
 */
struct vbe_mode_info {
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
} __attribute__ ((packed));

/**
 * Get the monitors EDID information
 * TODO: Add EDID/VBE resolution mode verification
 * @return The EDID information
 */
struct edid_data get_edid();

/**
 * Set the video mode to a specified resolution using
 * a video mode code
 * @param mode The requested video mode code from 0x4F00 call
 * @return A structure with information about the video mode
 */
struct vbe_mode_info *vbe_set_mode(unsigned short mode);

/**
 * Find the highest resolution using 0x4F00 and call
 * vbe_set_mode using the video_modes far_ptr
 */
void set_optimal_resolution();

/**
 * Draw a string in VESA mode
 * @param ch
 */
void vesa_draw_string(char *data);

/**
 * The current video mode
 */
int vbe_current_mode;

/**
 * The width of the current video mode
 */
int vbe_width;

/**
 * The height of the current video mode
 */
int vbe_height;

/**
 * The bits per pixel (pixel width) of the current video mode
 */
int vbe_bpp;

/**
 * The pitch (bytes per line) of the current video mode
 */
int vbe_pitch;

/**
 * The framebuffer interface
 */
char *fb;

#endif
