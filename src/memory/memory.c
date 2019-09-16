#include "../graphics/graphics.h"

void *memory_copy(void *dest, const void *src, size_t count) {
    const char *sp = (const char *) src;
    char *dp = (char *) dest;
    for (; count != 0; count--) *dp++ = *sp++;
    return dest;
}

void *memory_set(void *dest, char val, size_t count) {
    char *temp = (char *) dest;
    for (; count != 0; count--) *temp++ = val;
    return dest;
}
