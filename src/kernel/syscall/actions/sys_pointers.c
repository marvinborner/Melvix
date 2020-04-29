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
	struct vbe_mode_info *ret = (struct vbe_mode_info *)umalloc(sizeof(struct vbe_mode_info));
	ret->attributes = current_mode_info->attributes;
	ret->pitch = current_mode_info->pitch;
	ret->width = current_mode_info->width;
	ret->height = current_mode_info->height;
	ret->bpp = current_mode_info->bpp;
	ret->memory_model = current_mode_info->memory_model;
	ret->framebuffer = current_mode_info->framebuffer;

	struct pointers *pointers = umalloc(sizeof(struct pointers));
	pointers->current_mode_info = ret;
	pointers->font = font;

	return pointers;
}