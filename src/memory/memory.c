#include "../graphics/vga.h"

unsigned char *memory_copy(unsigned char *dest, const unsigned char *src, int count) {
    // TODO: Add memory copy function
}

unsigned char *memory_set(unsigned char *dest, unsigned char val, int count) {
    unsigned char *p = dest;
    while (count > 0) {
        if (!*p) break;
        *p = val;
        p++;
        count--;
    }
    return dest;
}
