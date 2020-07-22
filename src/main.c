#include <def.h>
#include <vesa.h>

void main(u32 *mem_info, struct vid_info *vid_info)
{
	mem_info++; // TODO: Use the mmap!
	vbe = vid_info->info;

	u32 terminal_background[3] = { 0x1d, 0x1f, 0x24 };
	vesa_clear(terminal_background);

	while (1) {
	};
}
