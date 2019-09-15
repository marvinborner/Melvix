#ifndef MELVIX_MEMORY_H
#define MELVIX_MEMORY_H

#include <stddef.h>

void *memory_copy(void *dest, const void *src, size_t count);

void *memory_set(void *dest, char val, size_t count);

#endif
