#ifndef MELVIX_INITRD_H
#define MELVIX_INITRD_H

#include <stdint.h>
#include <kernel/fs/vfs.h>

typedef struct {
    uint32_t nfiles;
} initrd_header_t;

typedef struct {
    uint8_t magic;
    int8_t name[64];
    uint32_t offset;
    uint32_t length;
} initrd_file_header_t;

fs_node_t *initialise_initrd(uint32_t location);

void initrd_test();

#endif
