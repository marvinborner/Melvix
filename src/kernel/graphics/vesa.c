#include "vesa.h"
#include "graphics.h"
#include "../lib/lib.h"
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

vbe_mode_info *vbe_set_mode(unsigned short mode) {
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

        vbe_mode_info *vbe_info = (vbe_mode_info *) 0xA0000;

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

    vbe_mode_info vbe_info;
    return &vbe_info;
}

void set_optimal_resolution() {
    extern vbe_info *vbe_init_structure;
    regs16_t regs;
    regs.ax = 0x4F00;
    regs.di = vbe_init_structure;
    regs.es = 0xA000;
    int32(0x10, &regs);

    if (regs.ax != 0x004F) {
        switch_to_vga();
    }

    vbe_info *vbe_modes = (vbe_info *) 0xA0000;

    if (strcmp(vbe_modes->signature, "VESA") == 0) {
        init();
        terminal_write_string("SUCCESS!\n");
        keyboard_install();
    } else {
        init();
        terminal_write_string("FAILED!\n");
        keyboard_install();
    }
}