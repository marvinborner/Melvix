#include <stdint.h>
#include <kernel/graphics/vesa.h>
#include <kernel/fs/load.h>
#include <kernel/memory/alloc.h>
#include <kernel/memory/paging.h>
#include <kernel/tasks/process.h>

struct pointers {
	struct vbe_mode_info *current_mode_info;
	struct font *font;
};

uint32_t sys_pointers()
{
	struct pointers *pointers = umalloc(sizeof(struct pointers));
	pointers->current_mode_info = current_mode_info;
	pointers->font = font;

	return pointers;
}