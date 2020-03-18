#include <syscall.h>
#include <graphics/graphics.h>

unsigned char *fb;
int vbe_bpl = 4;
int vbe_pitch = 10240;
int vbe_height = 1600;
int vbe_width = 2560;

void vesa_draw_rectangle(int x1, int y1, int x2, int y2, const uint32_t color[3])
{
	int pos1 = x1 * vbe_bpl + y1 * vbe_pitch;
	char *draw = (char *)&fb[pos1];
	for (int i = 0; i <= y2 - y1; i++) {
		for (int j = 0; j <= x2 - x1; j++) {
			draw[vbe_bpl * j] = color[2];
			draw[vbe_bpl * j + 1] = color[1];
			draw[vbe_bpl * j + 2] = color[0];
		}
		draw += vbe_pitch;
	}
}

void vesa_set_pixel(uint16_t x, uint16_t y, const uint32_t color[3])
{
	unsigned pos = x * vbe_bpl + y * vbe_pitch;
	char *draw = (char *)&fb[pos];
	draw[pos] = color[2];
	draw[pos + 1] = color[1];
	draw[pos + 2] = color[0];
}

void vesa_clear()
{
	uint32_t color[3] = { 0, 0, 0 };
	vesa_draw_rectangle(0, 0, vbe_width - 1, vbe_height - 1, color);
}

void init_framebuffer()
{
	struct userspace_pointers *pointers = (struct userspace_pointers *)syscall_get_pointers();
	fb = (unsigned char *)0xfd000000;

	uint32_t color[3] = { 0xff, 0x00, 0x00 };
	vesa_set_pixel(0, 0, color);
}