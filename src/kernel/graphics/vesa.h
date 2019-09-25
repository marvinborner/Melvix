#ifndef MELVIX_VESA_H
#define MELVIX_VESA_H

#include <stdint.h>

typedef struct __attribute__ ((packed)) {
    char signature[4];
    uint32_t version;
    uint32_t oem;
    uint32_t capabilities;
    uint32_t video_modes;
    uint32_t video_memory;
    uint32_t software_rev;
    uint32_t vendor;
    uint32_t product_name;
    uint32_t product_rev;
    char reserved[222];
    char oem_data[256];
} vbe_info;

typedef struct __attribute__ ((packed)) {
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
} vbe_mode_info;

vbe_mode_info *vbe_set_mode(unsigned short mode);

void set_optimal_resolution();

typedef struct __attribute__ ((packed)) {
    unsigned short di, si, bp, sp, bx, dx, cx, ax;
    unsigned short gs, fs, es, ds, eflags;
} regs16_t;

extern void int32(unsigned char intnum, regs16_t *regs);

int vbe_current_mode;
int vbe_width;
int vbe_height;
int vbe_bpp;
int vbe_pitch;

#endif
