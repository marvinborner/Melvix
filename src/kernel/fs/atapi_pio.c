#include <stdint.h>
#include <kernel/fs/atapi_pio.h>
#include <kernel/system.h>
#include <kernel/paging/paging.h>

void ATAPI_read(uint16_t nblocks, uint32_t lba)
{
    struct dapack *d = (struct dapack *) ATAPI_PIO_DAPACK;
    d->size = 0x10;
    d->null = 0x00;
    d->blk_count = nblocks;
    d->b_offset = ATAPI_PIO_BUFFER;
    d->b_segment = 0x0000;
    d->start = lba;
    d->upper_lba_bits = 0x00000000;

    regs16_t regs;
    regs.ax = 0x4200;
    regs.dx = ATAPI_PIO_DRIVE;
    regs.ds = 0;
    regs.si = ATAPI_PIO_DAPACK;

    v86(LBA_READ_INT, &regs);
}

void ATAPI_granular_read(uint32_t nblocks, uint32_t lba, uint8_t *output)
{
    for (uint32_t i = 0; i < nblocks; i++) {
        ATAPI_read(1, lba + i);
        for (uint16_t j = 0; j < ATAPI_SECTOR_SIZE; j++)
            output[j + (2048 * i)] = ((uint8_t *) ATAPI_PIO_BUFFER)[j];
    }
}