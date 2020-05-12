#ifndef ATA_DRIVER_H
#define ATA_DRIVER_H

#include <stdint.h>

#define SECTOR_SIZE 512

#define LBA_BITS 28

// Port bases and offsets
// TODO: Support other emulators/devices by using PCI ATA detection
#define PRIMARY_BASE 0x1F0
#define SECONDARY_BASE 0x170
#define DATA 0
#define ERROR 1
#define SECTOR_COUNT 2
#define LBA_LOW 3
#define LBA_MID 4
#define LBA_HIGH 5
#define DRIVE_SELECT 6
#define COM_STAT 7

#define PRI_CONTROL 0x3F6
#define SEC_CONTROL 0x376

// Commands
#define SEL_MASTER 0xA0
#define SEL_SLAVE 0xB0
#define IDENTIFY 0xEC
#define READ_SECTORS 0x20

// Status byte flags
#define ERR (1 << 0)
#define DRQ (1 << 3)
#define SRV (1 << 4)
#define DF (1 << 5)
#define RDY (1 << 6)
#define BSY (1 << 7)

#define MAX_28LBA_SECTORS 60

void ata_init();

void read_abs_sectors(u32 lba, u8 sector_count, u16 buf[]);

#endif