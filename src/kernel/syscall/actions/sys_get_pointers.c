#include <stdint.h>
#include <kernel/graphics/vesa.h>
#include <kernel/graphics/font.h>
#include <kernel/paging/paging.h>
#include <kernel/io/io.h>
#include <kernel/lib/stdlib/liballoc.h>

struct userspace_pointers {
    unsigned char *fb;
    struct font *font;
};

uint32_t sys_get_pointers()
{
    struct userspace_pointers *pointers = (struct userspace_pointers *) umalloc(sizeof(struct userspace_pointers));
    pointers->fb = fb;
    pointers->font = font;
    return (uint32_t) pointers;
}