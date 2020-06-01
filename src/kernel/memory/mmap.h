#ifndef MELVIX_MMAP
#define MELVIX_MMAP

#include <stdint.h>

void *kmalloc_frames(u32 num);
void kfree_frames(void *ptr, u32 num);

u8 mmap_check(u32 n);
void mmap_init(u32 size);
void mmap_init_finalize();
void mmap_address_set_free(u32 address);
void mmap_address_set_used(u32 address);
u8 mmap_index_check(u32 n);
void mmap_index_set_free(u32 n);
void mmap_index_set_used(u32 n);

#endif