#ifndef MELVIX_ALLOC_H
#define MELVIX_ALLOC_H

#include <stddef.h>

#define PREFIX(func) k ## func

int liballoc_lock();

int liballoc_unlock();

void *liballoc_alloc(size_t);

int liballoc_free(void *, size_t);

void *PREFIX(malloc)(size_t);

void *PREFIX(realloc)(void *, size_t);

void *PREFIX(calloc)(size_t, size_t);

void PREFIX(free)(void *);

#endif
