#include "vesa.h"
#include "graphics.h"
#include "../system.h"
#include "../lib/lib.h"
#include "../paging/kheap.h"
#include "../io/io.h"
#include "font.h"

void switch_to_vga() {
    write_serial("Force switch to VGA!");
    vesa_available = 0;
    regs16_t regs;
    regs.ax = 0x0003;
    int32(0x10, &regs);
}

struct edid_data get_edid() {
    struct edid_data *edid = umalloc(sizeof(struct edid_data));

    regs16_t regs;
    regs.ax = 0x4F15;
    regs.bx = 0x01; // BL
    regs.es = get_segment(edid);
    regs.di = get_offset(edid);
    int32(0x10, &regs);

    ufree(edid);

    return *edid;
}

struct vbe_mode_info *vbe_set_mode(unsigned short mode) {
    write_serial("Setting VBE mode!");
    vesa_available = 0;
    regs16_t regs;
    regs.ax = 0x4F02;
    regs.bx = mode | (1 << 14);
    int32(0x10, &regs);

    if (regs.ax == 0x004F) {
        regs.ax = 0x4F01;
        regs.cx = mode;
        regs.es = 0xA000;
        regs.di = 0x0000;
        int32(0x10, &regs);
        if (regs.ax != 0x004F) {
            switch_to_vga();
            return ((void *) 0);
        }

        struct vbe_mode_info *vbe_info = (struct vbe_mode_info *) 0xA0000;

        terminal_write_number(vbe_info->width);
        terminal_write_number(vbe_info->height);
        vbe_width = vbe_info->width;
        vbe_height = vbe_info->height;
        vbe_bpp = vbe_info->bpp / 8;
        vbe_pitch = vbe_info->pitch;
        fb = (char *) vbe_info->framebuffer;

        /*for (int i = 0; i < vbe_width * vbe_height * vbe_bpp; i++) {
        fb[i] = 100;
        fb[i + 1] = 100;
        fb[i + 2] = 100;
        }*/

        vesa_available = 1;

        return vbe_info;
    } else {
        switch_to_vga();
    }

    struct vbe_mode_info vbe_info;
    return &vbe_info;
}

void set_optimal_resolution() {
    vesa_available = 0;
    struct vbe_info *info = (struct vbe_info *) 0x2000;
    struct vbe_mode_info *mode_info = (struct vbe_mode_info *) 0x3000;

    memory_copy(info->signature, "VBE2", 4);

    regs16_t regs;
    regs.ax = 0x4F00;
    regs.es = 0;
    regs.di = info;
    int32(0x10, &regs);

    if (regs.ax != 0x004F || strcmp(info->signature, "VESA") != 0) {
        switch_to_vga();
        return;
    }

    uint16_t *mode_ptr = get_ptr(info->video_modes);
    uint16_t mode;
    uint16_t highest = 0x11B;
    uint16_t highest_width = 0;

    while ((mode = *mode_ptr++) != 0xFFFF) {
        mode &= 0x1FF;
        regs16_t regs2;
        regs2.ax = 0x4F01;
        regs2.cx = mode;
        regs2.es = get_segment(mode_info);
        regs2.di = get_offset(mode_info);
        int32(0x10, &regs2);

        if ((mode_info->attributes & 0x90) != 0x90) continue;

        if (mode_info->width >= highest_width &&
            (float) mode_info->width / (float) mode_info->height < 2.0 &&
            (mode_info->attributes & 0x1) != 0x1 &&
            (mode_info->attributes & 0x90) != 0x90 &&
            mode_info->memory_model != 6) {
            highest = mode;
            highest_width = mode_info->width;
        }
    }

    vbe_set_mode(highest);
}

uint16_t terminal_x = 1;
uint16_t terminal_y = 1;
uint32_t terminal_color = 0xFFFFFF;

// char text[1024] = {0};

void vesa_clear() {
    for (int i = 0; i < vbe_width * vbe_height * vbe_bpp; i++) {
        fb[i] = 0;
        fb[i + 1] = 0;
        fb[i + 2] = 0;
    }
}

void vesa_set_pixel(uint16_t x, uint16_t y, uint32_t color) {
    unsigned pos = x * (vbe_bpp / 8) + y * vbe_pitch;
    fb[pos] = color & 255;
    fb[pos + 1] = (color >> 8) & 255;
    fb[pos + 2] = (color >> 16) & 255;
}

void vesa_draw_char(char ch, int x, int y) {
    int cx, cy;
    int mask[8] = {1, 2, 4, 8, 16, 32, 64, 128};
    unsigned char *glyph = font[ch - 32];

    for (cy = 0; cy < 13; cy++) {
        for (cx = 0; cx < 8; cx++) {
            if (glyph[cy] & mask[cx]) {
                vesa_set_pixel(x + 8 - cx, y + 13 - cy, terminal_color);
            }
        }
    }
}

void vesa_draw_string(char *data) {
    vesa_clear();
    int i = 0;
    while (data[i] != '\0') {
        vesa_draw_char(data[i], terminal_x + (10 * i), terminal_y);
        i++;
    }
}

void vesa_set_color(uint32_t color) {
    terminal_color = color;
}