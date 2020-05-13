#ifndef MELVIX_ALLOC_H
#define MELVIX_ALLOC_H

#include <stdint.h>

void *malloc(u32);
void *realloc(void *, u32);
void *calloc(u32, u32);
void free(void *);

#endif