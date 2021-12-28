/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvIO_INBorner.de>
 * SPDX-License-Identifier: MIT
 */
// ATA/ATAPI is the standard for data transfer
// TODO: Reduce magic numbers

#include <assert.h>
#include <err.h>

#include <core/io.h>
#include <ports/ata.h>

#define BLOCK_SIZE 1024
#define BLOCK_COUNT (BLOCK_SIZE / sizeof(u32)) // 256
#define SECTOR_SIZE 512
#define SECTOR_COUNT (BLOCK_SIZE / SECTOR_SIZE)

#define PRIMARY_IO 0x1f0
#define SECONDARY_IO 0x170

// From spec
#define PRIMARY 0x00
#define SECONDARY 0x01
#define READ 0x00
#define WRITE 0x013
#define MASTER 0x00
#define SLAVE 0x01
#define SR_BSY 0x80
#define SR_DRDY 0x40
#define SR_DF 0x20
#define SR_DSC 0x10
#define SR_DRQ 0x08
#define SR_CORR 0x04
#define SR_IDX 0x02
#define SR_ERR 0x01
#define REG_DATA 0x00
#define REG_ERROR 0x01
#define REG_FEATURES 0x01
#define REG_SECCOUNT0 0x02
#define REG_LBA0 0x03
#define REG_LBA1 0x04
#define REG_LBA2 0x05
#define REG_HDDEVSEL 0x06
#define REG_COMMAND 0x07
#define REG_STATUS 0x07
#define REG_SECCOUNT1 0x08
#define REG_LBA3 0x09
#define REG_LBA4 0x0a
#define REG_LBA5 0x0b
#define REG_CONTROL 0x0c
#define REG_ALTSTATUS 0x0c
#define REG_DEVADDRESS 0x0d
#define CMD_READ_PIO 0x20
#define CMD_READ_PIO_EXT 0x24
#define CMD_READ_DMA 0xc8
#define CMD_READ_DMA_EXT 0x25
#define CMD_WRITE_PIO 0x30
#define CMD_WRITE_PIO_EXT 0x34
#define CMD_WRITE_DMA 0xca
#define CMD_WRITE_DMA_EXT 0x35
#define CMD_CACHE_FLUSH 0xe7
#define CMD_CACHE_FLUSH_EXT 0xea
#define CMD_PACKET 0xa0
#define CMD_IDENTIFY_PACKET 0xa1
#define CMD_IDENTIFY 0xec
#define IDENT_DEVICETYPE 0
#define IDENT_CYLINDERS 2
#define IDENT_HEADS 6
#define IDENT_SECTORS 12
#define IDENT_SERIAL 20
#define IDENT_MODEL 54
#define IDENT_CAPABILITIES 98
#define IDENT_FIELDVALID 106
#define IDENT_MAX_LBA 120
#define IDENT_COMMANDSETS 164
#define IDENT_MAX_LBA_EXT 200

static void select_drive(u8 bus, u8 drive)
{
	if (bus == PRIMARY) {
		if (drive == MASTER)
			IO_OUTB(PRIMARY_IO + REG_HDDEVSEL, 0xa0);
		else if (drive == SLAVE)
			IO_OUTB(PRIMARY_IO + REG_HDDEVSEL, 0xb0);
		else
			panic("Invalid drive %d", drive);
	} else if (bus == SECONDARY) {
		if (drive == MASTER)
			IO_OUTB(SECONDARY_IO + REG_HDDEVSEL, 0xa0);
		else if (drive == SLAVE)
			IO_OUTB(SECONDARY_IO + REG_HDDEVSEL, 0xb0);
		else
			panic("Invalid drive %d", drive);
	} else {
		panic("Invalid bus %d", bus);
	}
}

static err exists(u8 bus, u8 drive)
{
	u16 io = bus == PRIMARY ? PRIMARY_IO : SECONDARY_IO;
	select_drive(bus, drive);

	// Reset
	IO_OUTB(io + REG_SECCOUNT0, 0);
	IO_OUTB(io + REG_LBA0, 0);
	IO_OUTB(io + REG_LBA1, 0);
	IO_OUTB(io + REG_LBA2, 0);

	// Identify
	IO_OUTB(io + REG_COMMAND, CMD_IDENTIFY);
	u8 status = IO_INB(io + REG_STATUS);
	if (!status)
		return -ERR_NOT_FOUND;

	while ((IO_INB(io + REG_STATUS) & SR_BSY))
		;

	do {
		status = IO_INB(io + REG_STATUS);
		if (status & SR_ERR)
			return -ERR_HARDWARE;
	} while ((status & SR_DRQ) == 0);

	static u8 ide_buf[SECTOR_SIZE] = { 0 };
	for (u32 i = 0; i < BLOCK_COUNT; i++)
		*(u16 *)(ide_buf + i * 2) = IO_INW(io + REG_DATA);

	// TODO: Do something with ide_buf (for disk management?)

	return ERR_OK;
}

static void poll(u16 io)
{
	while (IO_INB(io + REG_STATUS) & SR_BSY)
		;

	assert(!(IO_INB(io + REG_STATUS) & SR_ERR));
}

// No standard read operation though
static err read(u8 bus, u8 drive, void *buf, u32 lba, u32 sector_count)
{
	u16 io = bus == PRIMARY ? PRIMARY_IO : SECONDARY_IO;
	u8 cmd = drive == MASTER ? 0xe0 : 0xf0;

	IO_OUTB(io + REG_HDDEVSEL, (cmd | (u8)((lba >> 24 & 0x0f))));
	IO_OUTB(io + REG_FEATURES, 0);
	IO_OUTB(io + REG_SECCOUNT0, sector_count);
	IO_OUTB(io + REG_LBA0, (u8)lba);
	IO_OUTB(io + REG_LBA1, (u8)(lba >> 8));
	IO_OUTB(io + REG_LBA2, (u8)(lba >> 16));
	IO_OUTB(io + REG_COMMAND, CMD_READ_PIO);

	u16 *b = buf;
	u32 count = sector_count;
	while (count-- > 0) {
		poll(io);
		__asm__ volatile("rep insw" ::"c"(BLOCK_COUNT), "d"(io + REG_DATA), "D"((u32)b));
	}

	/* ide_delay(io); */
	return sector_count;
}

static err request(u32 request, va_list ap)
{
	switch (request) {
	case PORT_ATA_EXISTS: {
		u8 bus = va_arg(ap, int);
		u8 drive = va_arg(ap, int);
		return exists(bus, drive);
	}
	case PORT_ATA_READ: {
		u8 bus = va_arg(ap, int);
		u8 drive = va_arg(ap, int);
		void *buf = va_arg(ap, void *);
		u32 lba = va_arg(ap, int);
		u32 sector_count = va_arg(ap, int);
		return read(bus, drive, buf, lba, sector_count);
	}
	default:
		return -ERR_INVALID_ARGUMENTS;
	}
}

static err probe(void)
{
	// TODO: Do some self-test kind of thing
	return ERR_OK;
}

static err setup(void)
{
	return ERR_OK;
}

PROTECTED struct port port_ata = {
	.type = PORT_ATA,
	.request = request,
	.probe = probe,
	.setup = setup,
};
