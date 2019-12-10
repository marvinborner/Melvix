#ifndef MELVIX_ALLOC_H
#define MELVIX_ALLOC_H

#include <stddef.h>

int liballoc_lock();

int liballoc_unlock();

void *liballoc_alloc(size_t);

int liballoc_free(void *, size_t);

void *malloc(size_t);

void *realloc(void *, size_t);

void *calloc(size_t, size_t);

void free(void *);

#endif
