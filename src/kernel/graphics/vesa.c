#include "vesa.h"

extern struct vbe_best vbe_find_mode();

extern void vbe_set_mode(struct vbe_best);

struct vbe_best best;

void init_graphics() {
    best = vbe_find_mode();
}