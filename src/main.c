#include <def.h>

void clear();

// This must kinda be at the top
int main(u32 *mem_info)
{
	mem_info++; // TODO: Use the mmap!
	clear();
	while (1) {
	};
	return 0;
}

void clear()
{
	char *vga = (char *)0x000B8000;
	for (long i = 0; i < 80 * 25; i++) {
		*vga++ = 0;
		*vga++ = 0;
	}
}
