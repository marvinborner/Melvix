#include <syscall.h>
#include <graphics/graphics.h>

unsigned char *fb;
int vbe_bpl = 3;
int vbe_pitch = 3000;
int vbe_height = 1080;
int vbe_width = 2560;

void vesa_draw_rectangle(int x1, int y1, int x2, int y2, const uint32_t color[3])
{
    int pos1 = x1 * vbe_bpl + y1 * vbe_pitch;
    char *draw = (char *) &fb[pos1];
    for (int i = 0; i <= y2 - y1; i++) {
        for (int j = 0; j <= x2 - x1; j++) {
            draw[vbe_bpl * j] = color[2];
            draw[vbe_bpl * j + 1] = color[1];
            draw[vbe_bpl * j + 2] = color[0];
        }
        draw += vbe_pitch;
    }
}

void vesa_clear()
{
    vesa_draw_rectangle(0, 0, vbe_width - 1, vbe_height - 1, 0);
}

void init_framebuffer()
{
    struct userspace_pointers *pointers = (struct userspace_pointers *) syscall_get_pointers();
    fb = pointers->fb;
    vesa_clear();
}