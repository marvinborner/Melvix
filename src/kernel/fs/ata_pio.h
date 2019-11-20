#ifndef MELVIX_ATA_PIO_H
#define MELVIX_ATA_PIO_H

#include <stdint.h>

#define BYTES_PER_SECTOR 512

struct ATA_INTERFACE {
    uint8_t master;
    uint16_t dataPort;
    uint16_t errorPort;
    uint16_t sectorCountPort;
    uint16_t lbaLowPort;
    uint16_t lbaMidPort;
    uint16_t lbaHiPort;
    uint16_t devicePort;
    uint16_t commandPort;
    uint16_t controlPort;
};

struct ATA_INTERFACE *new_ATA(uint8_t master, uint16_t portBase);

uint8_t ATA_identify(struct ATA_INTERFACE *iface, uint16_t *retdata);

uint8_t *ATA_read28(struct ATA_INTERFACE *iface, uint32_t sector);

uint8_t ATA_write28(struct ATA_INTERFACE *iface, uint32_t sector, uint8_t *contents);

uint8_t ATA_clear28(struct ATA_INTERFACE *iface, uint32_t sector);

#endif
