#include <stdint.h>
#include <kernel/graphics/vesa.h>
#include <kernel/fs/load.h>
#include <kernel/memory/alloc.h>

struct pointers {
	uint8_t *fb;
	struct font *font;
};

uint32_t sys_pointers()
{
	struct pointers *pointers = kmalloc(sizeof(struct pointers));
	pointers->fb = fb;
	pointers->font = font;

	return pointers;
}