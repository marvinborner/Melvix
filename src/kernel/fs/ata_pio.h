#ifndef MELVIX_ATA_PIO_H
#define MELVIX_ATA_PIO_H

#include <stdint.h>

#define BYTES_PER_SECTOR 512

struct ata_interface {
    uint8_t master;
    uint16_t data_port;
    uint16_t error_port;
    uint16_t sector_count_port;
    uint16_t lba_low_port;
    uint16_t lba_mid_port;
    uint16_t lba_high_port;
    uint16_t device_port;
    uint16_t command_port;
    uint16_t control_port;
};

struct ata_interface *new_ata(uint8_t master, uint16_t port_base);

uint8_t ata_identify(struct ata_interface *interface, uint16_t *ret_data);

uint8_t *ata_read28(struct ata_interface *interface, uint32_t sector);

uint8_t ata_write28(struct ata_interface *interface, uint32_t sector, const uint8_t *contents);

uint8_t ata_clear28(struct ata_interface *interface, uint32_t sector);

#endif
