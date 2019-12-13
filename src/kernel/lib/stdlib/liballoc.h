#ifndef MELVIX_ALLOC_H
#define MELVIX_ALLOC_H

#include <stddef.h>

int liballoc_lock();

int liballoc_unlock();

void *liballoc_alloc(size_t);

int liballoc_free(void *, size_t);

void *kmalloc(size_t);

void *krealloc(void *, size_t);

void *kcalloc(size_t, size_t);

void kfree(void *);

void *umalloc(size_t);

void *urealloc(void *, size_t);

void *ucalloc(size_t, size_t);

void ufree(void *);

#endif
