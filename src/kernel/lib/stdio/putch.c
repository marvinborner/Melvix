#include <kernel/graphics/vesa.h>

void putch(char c)
{
	vesa_draw_char(c);
}