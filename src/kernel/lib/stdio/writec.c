#include <kernel/graphics/vesa.h>

void writec(char c)
{
	vesa_draw_char(c);
}