#include "vesa.h"

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
            // Add VGA redirect
        }

        vbe_mode_info *vbe_info = (vbe_mode_info *) 0xA0000;

        vbe_width = vbe_info->width;
        vbe_height = vbe_info->height;
        vbe_bpp = vbe_info->bpp / 8;
        vbe_pitch = vbe_info->pitch;

        char *fb = (char *) vbe_info->framebuffer;
        for (int i = 0; i < 640 * 480 * 3; i++) {
            fb[i] = 100;
        }
        regs.ax = 0x0000;
        int32(0x16, &regs);
        regs.ax = 0x0003;
        int32(0x10, &regs);

        return vbe_info;
    } else {
        // Add VGA redirect
    }

    vbe_mode_info vbe_info;
    return &vbe_info;
}
