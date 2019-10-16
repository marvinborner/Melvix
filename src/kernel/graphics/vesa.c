#include "vesa.h"
#include "../io/io.h"
#include "../lib/lib.h"
#include "../paging/kheap.h"
#include "../paging/paging.h"
#include "../system.h"
#include "font.h"

extern page_directory_t *kernel_directory;

void switch_to_vga() {
    serial_write("Force switch to VGA!\n");
    vesa_available = 0;
    regs16_t regs;
    regs.ax = 0x0003;
    disable_paging();
    int32(0x10, &regs);
    enable_paging();

    uint16_t *terminal_buffer = (uint16_t *) 0xB8000;
    char *error = "This computer has no supported video drivers!";
    for (size_t i = 0; i < strlen(error); i++)
        terminal_buffer[i] = (uint16_t) error[i] | (uint16_t) 0x700;
    panic("No VESA!");
}

struct edid_data get_edid() {
    struct edid_data *edid = (struct edid_data *) kmalloc(sizeof(struct edid_data));

    regs16_t regs;
    regs.ax = 0x4F15;
    regs.bx = 0x01; // BL
    regs.es = get_segment(edid);
    regs.di = get_offset(edid);
    disable_paging();
    int32(0x10, &regs);
    enable_paging();

    kfree(edid);

    return *edid;
}

void vbe_set_mode(unsigned short mode) {
    vesa_available = 0;
    regs16_t regs;
    regs.ax = 0x4F02;
    regs.bx = mode;
    regs.bx |= 0x4000;
    regs.bx &= 0x7FFF;
    disable_paging();
    int32(0x10, &regs);
    enable_paging();

    if (regs.ax != 0x004F) switch_to_vga();
    else vesa_available = 1;
}

uint16_t *vbe_get_modes() {
    vesa_available = 0;
    char *info_address = (char *) 0x7E00;
    strcpy(info_address, "VBE2");
    for (int i = 4; i < 512; i++) *(info_address + i) = 0;

    regs16_t regs;
    regs.ax = 0x4F00;
    regs.es = 0;
    regs.di = 0x7E00;
    disable_paging();
    int32(0x10, &regs);
    enable_paging();

    struct vbe_info *info = (struct vbe_info *) info_address;

    if (regs.ax != 0x004F || strcmp(info->signature, "VESA") != 0) {
        switch_to_vga();
        return ((void *) 0);
    }

    // Get number of modes
    uint16_t *mode_ptr = get_ptr(info->video_modes);
    int number_modes = 1;
    for (uint16_t *p = mode_ptr; *p != 0xFFFF; p++) number_modes++;

    uint16_t *video_modes = (uint16_t *) kmalloc(sizeof(uint16_t) * number_modes);
    for (int i = 0; i < number_modes; i++)
        video_modes[i] = mode_ptr[i];

    return video_modes;
}

struct vbe_mode_info *vbe_get_mode_info(uint16_t mode) {
    regs16_t regs;
    regs.ax = 0x4F01;
    regs.cx = mode;
    regs.es = 0;
    regs.di = 0x7E00;
    disable_paging();
    int32(0x10, &regs);
    enable_paging();

    struct vbe_mode_info_all *mode_info = (struct vbe_mode_info_all *) 0x7E00;
    struct vbe_mode_info *mode_info_final = (struct vbe_mode_info *) kmalloc(sizeof(struct vbe_mode_info));
    mode_info_final->attributes = mode_info->attributes;
    mode_info_final->width = mode_info->width;
    mode_info_final->height = mode_info->height;
    mode_info_final->bpp = mode_info->bpp;
    mode_info_final->framebuffer = mode_info->framebuffer;

    return mode_info_final;
}

void set_optimal_resolution() {
    uint16_t *video_modes = vbe_get_modes();
    uint16_t highest = 0x11B;
    uint16_t highest_width = 0;

    for (uint16_t *mode = video_modes; *mode != 0xFFFF; mode++) {
        struct vbe_mode_info *mode_info = vbe_get_mode_info(*mode);

        if (mode_info->width >= highest_width &&
            // (float) mode_info->width / (float) mode_info->height < 2.0 &&
            mode_info->attributes & 0x80) {
            highest = *mode;
            highest_width = mode_info->width;
            kfree(mode_info);
        }
        kfree(mode_info);
    }
    kfree(video_modes);

    struct vbe_mode_info *highest_info = vbe_get_mode_info(highest);
    vbe_width = highest_info->width;
    vbe_height = highest_info->height;
    vbe_bpp = highest_info->bpp >> 3;
    fb = highest_info->framebuffer;

    serial_write("Using mode: ");
    serial_write_dec(vbe_width);
    serial_write("x");
    serial_write_dec(vbe_height);
    serial_write("x");
    serial_write_dec(vbe_bpp << 3);
    serial_write("\n");

    uint32_t fb_size = vbe_width * vbe_height * vbe_bpp;
    for (uint32_t z = 0; z < fb_size; z += 4096)
        alloc_frame(get_page(fb + z, 1, kernel_directory), 1, 1);
    vbe_set_mode(highest);
}

uint16_t terminal_x = 1;
uint16_t terminal_y = 1;
uint32_t terminal_color = 0xBEBEBE;

// char text[1024] = {0};

void vesa_set_pixel(uint16_t x, uint16_t y, uint32_t color) {
    uint32_t pixel = x * vbe_bpp + y * vbe_width * vbe_bpp + fb;
    *((uint32_t *) pixel) = color;
}

void vesa_draw_char(char ch, int x, int y) {
    int mask[8] = {1, 2, 4, 8, 16, 32, 64, 128};
    unsigned char *glyph = font[ch - 32];

    for (int cy = 0; cy < 13; cy++) {
        for (int cx = 0; cx < 8; cx++) {
            if (glyph[cy] & mask[cx]) {
                vesa_set_pixel(x + 8 - cx, y + 13 - cy, terminal_color);
            }
        }
    }
}

void vesa_draw_string(char *data) {
    int i = 0;
    while (data[i] != '\0') {
        vesa_draw_char(data[i], terminal_x, terminal_y);
        terminal_x += 10;
        i++;
    }
}

void vesa_set_color(uint32_t color) {
    terminal_color = color;
}
