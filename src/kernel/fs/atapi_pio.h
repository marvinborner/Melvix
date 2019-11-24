#ifndef MELVIX_ATAPI_PIO_H
#define MELVIX_ATAPI_PIO_H

#include <stdint.h>

#define ATAPI_PIO_DRIVE 0xE0
#define LBA_READ_INT 0x13
#define ATAPI_PIO_DAPACK 0x7E00
#define ATAPI_PIO_BUFFER 0x8000
#define ATAPI_SECTOR_SIZE 0x800

struct dapack {
    uint8_t size;
    uint8_t null;
    uint16_t blk_count;
    uint16_t b_offset;
    uint16_t b_segment;
    uint32_t start;
    uint32_t upper_lba_bits;
} __attribute__((packed));

void ATAPI_read(uint16_t nblocks, uint32_t lba);

void ATAPI_granular_read(uint32_t nblocks, uint32_t lba, uint8_t *output);

#endif
