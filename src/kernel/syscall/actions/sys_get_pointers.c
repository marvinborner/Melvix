#include <stdint.h>
#include <kernel/graphics/vesa.h>
#include <kernel/graphics/font.h>
#include <kernel/paging/paging.h>

struct userspace_pointers {
    unsigned char *fb;
    struct font *font;
};

uint32_t sys_get_pointers()
{
    struct userspace_pointers *pointers = (struct userspace_pointers *) paging_alloc_pages(1);
    pointers->fb = fb;
    pointers->font = font;
    paging_set_user((uint32_t) pointers, 1);
    return (uint32_t) pointers;
}