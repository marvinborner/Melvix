#include <syscall.h>

char *fb;
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

void user_main()
{
    char hello[] = "> Successfully switched to usermode!\n";
    syscall_write(hello);

    // fb = (char *) 0x110048;
    // vesa_clear();

    while (1) {};

    /*while (1) {
        char *key = malloc(1);
        syscall_readc(key);
        syscall_writec(key);
    };*/
}