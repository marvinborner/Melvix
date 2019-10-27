#include <kernel/graphics/vesa.h>
#include <kernel/graphics/font.h>
#include <kernel/io/io.h>
#include <kernel/lib/lib.h>
#include <kernel/paging/paging.h>
#include <kernel/system.h>
#include <kernel/lib/alloc.h>
#include <kernel/commands/command.h>

void switch_to_vga() {
    serial_write("Force switch to VGA!\n");
    regs16_t regs;
    regs.ax = 0x0003;
    paging_disable();
    int32(0x10, &regs);
    paging_enable();

    uint16_t *terminal_buffer = (uint16_t *) 0xB8000;
    char *error = "Melvix does not support this graphics hardware!";
    for (size_t i = 0; i < strlen(error); i++)
        terminal_buffer[i] = (uint16_t) error[i] | (uint16_t) 0x700;
    panic("No VESA support!");
}

struct edid_data get_edid() {
    struct edid_data *edid = (struct edid_data *) kmalloc(sizeof(struct edid_data));

    regs16_t regs;
    regs.ax = 0x4F15;
    regs.bx = 0x01; // BL
    regs.es = 0;
    regs.di = (unsigned short) edid;
    paging_disable();
    int32(0x10, &regs);
    paging_enable();

    kfree(edid);

    return *edid;
}

void vbe_set_mode(unsigned short mode) {
    regs16_t regs;
    regs.ax = 0x4F02;
    regs.bx = mode;
    regs.bx |= 0x4000;
    paging_disable();
    int32(0x10, &regs);
    paging_enable();

    if (regs.ax != 0x004F)
        switch_to_vga();
}

uint16_t *vbe_get_modes() {
    char *info_address = (char *) 0x7E00;
    strcpy(info_address, "VBE2");
    for (int i = 4; i < 512; i++) *(info_address + i) = 0;

    regs16_t regs;
    regs.ax = 0x4F00;
    regs.es = 0;
    regs.di = 0x7E00;
    paging_disable();
    int32(0x10, &regs);
    paging_enable();

    struct vbe_info *info = (struct vbe_info *) info_address;

    if (regs.ax != 0x004F || strcmp(info->signature, "VESA") != 0) switch_to_vga();

    // Get number of modes
    uint16_t *mode_ptr = (uint16_t *) info->video_modes;
    size_t number_modes = 1;
    for (uint16_t *p = mode_ptr; *p != 0xFFFF; p++) number_modes++;

    return mode_ptr;
}

struct vbe_mode_info *vbe_get_mode_info(uint16_t mode) {
    regs16_t regs;
    regs.ax = 0x4F01;
    regs.cx = mode;
    regs.es = 0;
    regs.di = 0x7E00;
    paging_disable();
    int32(0x10, &regs);
    paging_enable();

    struct vbe_mode_info *mode_info = (struct vbe_mode_info *) 0x7E00;

    if (regs.ax != 0x004f) {
        return 0;
    }

    return mode_info;
}

void set_optimal_resolution() {
    vga_log("Switching to graphics mode", 9);
    uint16_t *video_modes = vbe_get_modes();

    uint16_t highest = 0;

    for (uint16_t *mode = video_modes; *mode != 0xFFFF; mode++) {
        struct vbe_mode_info *mode_info = vbe_get_mode_info(*mode);

        if (mode_info == 0 || (mode_info->attributes & 0x90) != 0x90 ||
            (mode_info->memory_model != 4 && mode_info->memory_model != 6))
            continue;

        serial_write("Found mode: (");
        serial_write_hex(*mode);
        serial_write(") ");
        serial_write_dec(mode_info->width);
        serial_write("x");
        serial_write_dec(mode_info->height);
        serial_write("x");
        serial_write_dec(mode_info->bpp);
        serial_write("\n");

        if (mode_info->width >= vbe_width) {
            // (float) mode_info->width / (float) mode_info->height < 2.0 &&) {
            highest = *mode;
            vbe_width = mode_info->width;
            vbe_height = mode_info->height;
            vbe_pitch = mode_info->pitch;
            vbe_bpp = mode_info->bpp >> 3;
            fb = (unsigned char *) mode_info->framebuffer;
        }
    }

    if (highest == 0) {
        serial_write("Mode detection failed!\nTrying common modes...\n");
        struct vbe_mode_info *mode_info;
        int modes[] = {
                322, 287, 286, 285, 284, // 1600x1200
                356, 355, 354, 353, 352, // 1440x900
                317, 283, 282, 281, 263, 262, // 1280x1024
                361, 360, 359, 358, 357, // 1152x720
                312, 280, 279, 278, 261, 260, // 1024x768
                366, 365, 364, 363, 362, // 1024x640
                307, 306, 305, 304, 303, // 896x672
                302, 277, 267, 275, 259, 258, 106, // 800x600
                371, 370, 369, 368, 367, // 800x500
                297, 274, 273, 272, 257, // 640x480
                292, 291, 290, 289, 256, // 640x400
                271, 270, 269, // 320x200
        };

        for (size_t i = 0; i < sizeof(modes) / sizeof(modes[0]); i++) {
            mode_info = vbe_get_mode_info(modes[i]);
            if (mode_info != 0 && mode_info->width > vbe_width) {
                highest = modes[i];
                vbe_width = mode_info->width;
                vbe_height = mode_info->height;
                vbe_pitch = mode_info->pitch;
                vbe_bpp = mode_info->bpp >> 3;
                fb = (unsigned char *) mode_info->framebuffer;
            }
        }

        // Everything else failed :(
        if (highest == 0)
            switch_to_vga();
    }

    vbe_set_mode(highest);

    vesa_clear();

    info("Successfully switched to video mode!");

    serial_write("Using mode: ");
    serial_write_hex(highest);
    log("Using mode: ");
    vesa_draw_number(vbe_width);
    vesa_draw_string("x");
    vesa_draw_number(vbe_height);
    vesa_draw_string("x");
    vesa_draw_number(vbe_bpp << 3);

    uint32_t fb_size = vbe_width * vbe_height * vbe_bpp;
    for (uint32_t z = 0; z < fb_size; z += 4096)
        paging_map((uint32_t) fb + z, (uint32_t) fb + z, PT_PRESENT | PT_RW | PT_USED);

}

uint16_t terminal_x = 1;
uint16_t terminal_y = 1;
uint32_t text_color = vesa_white;
uint32_t terminal_background = vesa_black;
uint32_t terminal_color = vesa_white;

void vesa_set_pixel(uint16_t x, uint16_t y, uint32_t color) {
    unsigned pos = x * vbe_bpp + y * vbe_pitch;
    fb[pos] = color & 255;
    fb[pos + 1] = (color >> 8) & 255;
    fb[pos + 2] = (color >> 16) & 255;
}

void vesa_draw_rectangle(int x1, int y1, int x2, int y2, int color) {
    int i, j;
    char blue = color & 255;
    char green = (color >> 8) & 255;
    char red = (color >> 16) & 255;
    int pos1 = x1 * vbe_bpp + y1 * vbe_pitch;
    char *draw = (char *) &fb[pos1];
    for (i = 0; i <= y2 - y1; i++) {
        for (j = 0; j <= x2 - x1; j++) {
            draw[vbe_bpp * j] = blue;
            draw[vbe_bpp * j + 1] = green;
            draw[vbe_bpp * j + 2] = red;
        }
        draw += vbe_pitch;
    }
}

void vesa_clear() {
    vesa_draw_rectangle(0, 0, vbe_width, vbe_height, terminal_background);
}

void vesa_draw_char(char ch) {
    if (ch >= ' ') {
        int mask[8] = {1, 2, 4, 8, 16, 32, 64, 128};
        unsigned char *glyph = font[ch - 32];

        for (int cy = 0; cy < 13; cy++) {
            for (int cx = 0; cx < 8; cx++) {
                if (glyph[cy] & mask[cx]) {
                    vesa_set_pixel(terminal_x + 8 - cx, terminal_y + 16 - cy, terminal_color);
                } else {
                    vesa_set_pixel(terminal_x + 8 - cx, terminal_y + 16 - cy, terminal_background);
                }
            }
        }

        terminal_x += 10;
    } else if (ch == '\n') {
        terminal_x = 0;
        terminal_y += 16;
    }

    if (terminal_x >= vbe_width) {
        terminal_x = 0;
        terminal_y += 16;
    }
}

void vesa_keyboard_char(char ch) {
    vesa_draw_rectangle(terminal_x, terminal_y, terminal_x + 8, terminal_y + 16, terminal_background);

    if (ch == 0x08) {
        if (terminal_x != 0) terminal_x -= 10;
    } else if (ch == 0x09) {
        terminal_x += 4 * 10;
    } else if (ch == '\r') {
        terminal_x = 0;
    } else if (ch == '\n') {
        vesa_draw_char(ch);
        exec_command(text);
        memory_set(text, 0, sizeof(text));
        // terminal_scroll();
    } else if (ch >= ' ') {
        vesa_draw_char(ch);
        strcat(text, &ch);
    }

    // terminal_scroll();
    vesa_draw_rectangle(terminal_x, terminal_y, terminal_x + 8, terminal_y + 16, terminal_color);
}

void vesa_draw_string(char *data) {
    int i = 0;
    while (data[i] != '\0') {
        vesa_draw_char(data[i]);
        i++;
    }
}

void vesa_draw_number(int n) {
    if (n == 0) vesa_draw_char('0');
    int acc = n;
    char c[32];
    int i = 0;
    while (acc > 0) {
        c[i] = '0' + acc % 10;
        acc /= 10;
        i++;
    }
    c[i] = 0;
    static char c2[32];
    c2[i--] = 0;
    int j = 0;
    while (i >= 0) c2[i--] = c[j++];
    vesa_draw_string(c2);
}

void vesa_set_color(uint32_t color) {
    terminal_color = color;
}
