#include "vesa.h"
#include "graphics.h"
#include "../input/input.h"
#include "../system.h"

void switch_to_vga() {
    regs16_t regs;
    regs.ax = 0x0003;
    int32(0x10, &regs);
    init();
    terminal_write_line("FAILED!");
    keyboard_install();
}

struct vbe_mode_info *vbe_set_mode(unsigned short mode) {
    regs16_t regs;
    regs.ax = 0x4F02;
    regs.bx = mode | (1 << 14);
    int32(0x10, &regs);

    if (regs.ax == 0x004F) {
        regs.ax = 0x4F01;
        regs.cx = mode;
        regs.di = 0x0000;
        regs.es = 0xA000;
        int32(0x10, &regs);
        if (regs.ax != 0x004F) {
            switch_to_vga();
        }

        struct vbe_mode_info *vbe_info = (struct vbe_mode_info *) 0xA0000;

        vbe_width = vbe_info->width;
        vbe_height = vbe_info->height;
        vbe_bpp = vbe_info->bpp / 8;
        vbe_pitch = vbe_info->pitch;

        char *fb = (char *) vbe_info->framebuffer;
        for (int i = 0; i < vbe_width * vbe_height * vbe_bpp; i++) {
            fb[i] = 100;
            fb[i + 1] = 100;
            fb[i + 2] = 100;
        }
        regs.ax = 0x0000;
        int32(0x16, &regs);
        regs.ax = 0x0003;
        int32(0x10, &regs);

        return vbe_info;
    } else {
        switch_to_vga();
    }

    struct vbe_mode_info vbe_info;
    return &vbe_info;
}

void set_optimal_resolution() {
    init();
    terminal_write_string("SUCCESS!\n");
    keyboard_install();

    struct vbe_info *info;
    struct vbe_mode_info *mode_info;

    // info = lmalloc(sizeof(struct vbe_info));
    // mode_info = lmalloc(sizeof(struct vbe_mode_info));

    info->signature[0] = 'V';
    info->signature[1] = 'B';
    info->signature[2] = 'E';
    info->signature[3] = '2';

    regs16_t regs;
    regs.ax = 0x4F00;
    regs.es = get_segment(info);
    regs.di = get_offset(info);
    int32(0x10, &regs);

    if (regs.ax != 0x004F) {
        switch_to_vga();
    }

    uint16_t *mode_ptr = get_ptr(info->video_modes);
    uint16_t mode;
    struct vbe_mode_info *highest;
    while ((mode = *mode_ptr++) != 0xFFFF) {
        mode &= 0x1FF;
        regs16_t regs2;
        regs2.ax = 0x4F01;
        regs2.cx = mode;
        regs2.es = get_segment(mode_info);
        regs2.di = get_offset(mode_info);
        int32(0x10, &regs2);

        if ((mode_info->attributes & 0x90) != 0x90) continue;
        if (mode_info->height >= highest->height) {
            highest = mode_info;
        }
    }

    // lfree(info);

    /*if (strcmp((const char *) info->version, (const char *) 0x300) == 0) {
        init();
        terminal_write_string("SUCCESS!\n");
        terminal_write_line((const char *) &info->vendor);
        keyboard_install();
    } else {
        init();
        terminal_write_string("FAILED!\n");
        keyboard_install(); // Find out why commands only work when keyboard gets reinstalled after write
    }*/
}