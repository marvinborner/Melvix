#include "vesa.h"
#include "graphics.h"
#include "../system.h"
#include "../lib/lib.h"
#include "../paging/kheap.h"
#include "../io/io.h"
#include "font.h"
#include "../paging/paging.h"

void switch_to_vga() {
    serial_write("Force switch to VGA!\n");
    vesa_available = 0;
    regs16_t regs;
    regs.ax = 0x0003;
    int32(0x10, &regs);
}

struct edid_data get_edid() {
    struct edid_data *edid = (struct edid_data *) kmalloc(sizeof(struct edid_data));

    regs16_t regs;
    regs.ax = 0x4F15;
    regs.bx = 0x01; // BL
    regs.es = get_segment(edid);
    regs.di = get_offset(edid);
    int32(0x10, &regs);

    kfree(edid);

    return *edid;
}

struct vbe_mode_info *vbe_set_mode(unsigned short mode) {
    serial_write("Setting VBE mode!\n");
    serial_write_hex(mode);
    vesa_available = 0;
    regs16_t regs;
    regs.ax = 0x4F02;
    regs.bx = mode | (1 << 14);
    disable_paging();
    int32(0x10, &regs);
    enable_paging();

    if (regs.ax == 0x004F) {
        struct vbe_mode_info *vbe_info = (struct vbe_mode_info *) 0x7E00;
        regs16_t regs2;
        regs2.ax = 0x4F01;
        regs2.cx = mode;
        regs2.es = 0;
        regs2.di = 0x7E00;
        disable_paging();
        int32(0x10, &regs2);
        enable_paging();

        if (regs2.ax != 0x004F) {
            switch_to_vga();
            return ((void *) 0);
        }

        struct vbe_mode_info *vbe_info_final = (struct vbe_mode_info *) kmalloc(sizeof(vbe_info));
        memory_copy(vbe_info_final, vbe_info, sizeof(vbe_info));

        vbe_width = vbe_info_final->width;
        vbe_height = vbe_info_final->height;
        vbe_bpp = vbe_info_final->bpp / 8;
        vbe_pitch = vbe_info_final->pitch;
        fb = (char *) vbe_info_final->framebuffer;

        /*for (int i = 0; i < vbe_width * vbe_height * vbe_bpp; i++) {
        fb[i] = 100;
        fb[i + 1] = 100;
        fb[i + 2] = 100;
        }*/

        vesa_available = 1;

        return vbe_info;
    } else {
        switch_to_vga();
        return ((void *) 0);
    }
}

uint16_t *vbe_get_modes() {
    vesa_available = 0;
    struct vbe_info *info = (struct vbe_info *) 0x7E00;

    memory_copy(info->signature, "VBE2", 4);

    regs16_t regs;
    regs.ax = 0x4F00;
    regs.es = 0;
    regs.di = 0x7E00;
    disable_paging();
    int32(0x10, &regs);
    enable_paging();

    if (regs.ax != 0x004F || strcmp(info->signature, "VESA") != 0) {
        switch_to_vga();
        return ((void *) 0);
    }

    uint16_t *mode_ptr = get_ptr(info->video_modes);
    int number_modes = 1;
    for (uint16_t *p = mode_ptr; *p != 0xFFFF; p++) number_modes++;

    uint16_t *video_modes = (uint16_t *) kmalloc(sizeof(uint16_t) * number_modes);
    for (int i = 0; i < number_modes; i++) video_modes[i] = mode_ptr[i];

    return video_modes;
}

void set_optimal_resolution() {
    uint16_t *video_modes = vbe_get_modes();
    uint16_t mode;
    uint16_t highest = 0x11B;
    uint16_t highest_width = 0;

    while ((mode = *video_modes++) != 0xFFFF) {
        mode &= 0x1FF;
        regs16_t regs2;
        regs2.ax = 0x4F01;
        regs2.cx = mode;
        regs2.es = 0;
        regs2.di = 0x7E00;
        disable_paging();
        int32(0x10, &regs2);
        enable_paging();

        struct vbe_mode_info *mode_info = (struct vbe_mode_info *) 0x7E00;

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
    serial_write("Reached set mode!\n");
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
    unsigned pos = x * (vbe_bpp) + y * vbe_pitch;
    fb[pos] = color & 255;
    fb[pos + 1] = (color >> 8) & 255;
    fb[pos + 2] = (color >> 16) & 255;
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

void vesa_draw_rectangle(int x1, int y1, int x2, int y2, int color) {
    char blue = color & 255;
    char green = (color >> 8) & 255;
    char red = (color >> 16) & 255;
    int pos1 = x1 * vbe_bpp + y1 * vbe_pitch;
    char *draw = &fb[pos1];
    for (int i = 0; i <= y2 - y1; i++) {
        for (int j = 0; j <= x2 - x1; j++) {
            draw[vbe_bpp * j] = blue;
            draw[vbe_bpp * j + 1] = green;
            draw[vbe_bpp * j + 2] = red;
        }
        draw += vbe_pitch;
    }
}

void vesa_draw_string(char *data) {
    vesa_clear();
    int i = 0;
    while (data[i] != '\0') {
        vesa_draw_char(data[i], terminal_x, terminal_y);
        terminal_x += 10;
        i++;
    }
    // vesa_draw_rectangle(terminal_x, terminal_y, terminal_x + 10, terminal_y + 16, 0xffffff);
}

void vesa_set_color(uint32_t color) {
    terminal_color = color;
}