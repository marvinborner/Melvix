#include <stdint.h>
#include <stdio.h>
#include <syscall.h>
#include <gui.h>

struct pointers *pointers;

u32 terminal_color[3] = { 0xab, 0xb2, 0xbf };
u32 terminal_background[3] = { 0x1d, 0x1f, 0x24 };

void gui_init()
{
	pointers = syscall_pointers();

	vbe_width = pointers->mode_info->width;
	vbe_height = pointers->mode_info->height;
	vbe_pitch = pointers->mode_info->pitch;
	vbe_bpl = pointers->mode_info->bpp >> 3;

	// TODO: Why tf is the kheap magic stored in the first few bytes?!
	fb = (pointers->mode_info->framebuffer << 16);

	/* gui_screen_clear(); */
	/* printf("%dx%dx%d\n", vbe_width, vbe_height, vbe_bpl << 3); */
	/* printf("0x%x\n", fb); */
}