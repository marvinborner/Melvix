// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <drivers/cpu.h>
#include <def.h>
#include <fs.h>
#include <drivers/ide.h>
#include <drivers/mbr.h>
#include <mem.h>
#include <drivers/pci.h>
#include <print.h>
#include <str.h>

PROTECTED static u8 *ide_buf = NULL;

struct ata_data {
	u8 drive;
};

CLEAR static void ide_select_drive(u8 bus, u8 drive)
{
	if (bus == ATA_PRIMARY) {
		if (drive == ATA_MASTER)
			outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xa0);
		else
			outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xb0);
	} else {
		if (drive == ATA_MASTER)
			outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, 0xa0);
		else
			outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, 0xb0);
	}
}

CLEAR static u8 ide_find(u8 bus, u8 drive)
{
	u16 io = bus == ATA_PRIMARY ? ATA_PRIMARY_IO : ATA_SECONDARY_IO;
	ide_select_drive(bus, drive);

	// Reset
	outb(io + ATA_REG_SECCOUNT0, 0);
	outb(io + ATA_REG_LBA0, 0);
	outb(io + ATA_REG_LBA1, 0);
	outb(io + ATA_REG_LBA2, 0);

	// Identify
	outb(io + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
	u8 status = inb(io + ATA_REG_STATUS);
	if (!status)
		return 0;

	while ((inb(io + ATA_REG_STATUS) & ATA_SR_BSY) != 0)
		;

	do {
		status = inb(io + ATA_REG_STATUS);
		if (status & ATA_SR_ERR)
			return 0;
	} while ((status & ATA_SR_DRQ) == 0);

	for (int i = 0; i < BLOCK_COUNT; i++)
		*(u16 *)(ide_buf + i * 2) = inw(io + ATA_REG_DATA);

	return 1;
}

static void ide_delay(u16 io) // 400ns
{
	for (int i = 0; i < 4; i++)
		inb(io + ATA_REG_ALTSTATUS);
}

static void ide_poll(u16 io)
{
	while (inb(io + ATA_REG_STATUS) & ATA_SR_BSY)
		;

	assert(!(inb(io + ATA_REG_STATUS) & ATA_SR_ERR));
}

static res ata_read(void *buf, u32 lba, u32 sector_count, struct vfs_dev *dev)
{
	u8 drive = ((struct ata_data *)dev->data)->drive;
	u16 io = (drive & ATA_PRIMARY << 1) == ATA_PRIMARY ? ATA_PRIMARY_IO : ATA_SECONDARY_IO;
	drive = (drive & ATA_SLAVE) == ATA_SLAVE ? ATA_SLAVE : ATA_MASTER;
	u8 cmd = drive == ATA_MASTER ? 0xe0 : 0xf0;

	outb(io + ATA_REG_HDDEVSEL, (cmd | (u8)((lba >> 24 & 0x0f))));
	outb(io + ATA_REG_FEATURES, 0);
	outb(io + ATA_REG_SECCOUNT0, sector_count);
	outb(io + ATA_REG_LBA0, (u8)lba);
	outb(io + ATA_REG_LBA1, (u8)(lba >> 8));
	outb(io + ATA_REG_LBA2, (u8)(lba >> 16));
	outb(io + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

	u16 *b = buf;
	u32 count = sector_count;
	while (count-- > 0) {
		ide_poll(io);
		__asm__ volatile("rep insw" ::"c"(BLOCK_COUNT), "d"(io + ATA_REG_DATA),
				 "D"((u32)b));
	}

	ide_delay(io);
	return sector_count;
}

u8 ata_pm = 0, ata_ps = 0, ata_sm = 0, ata_ss = 0;
CLEAR static void ata_probe(void)
{
	for (u8 i = 0; i < 4; i++) {
		u32 bus = i < 2 ? ATA_PRIMARY : ATA_SECONDARY;
		u32 drive = i % 2 ? ATA_MASTER : ATA_SLAVE;

		if (!ide_find(bus, drive))
			continue;

		struct ata_data *data = zalloc(sizeof(*data));
		data->drive = (bus << 1) | drive;

		struct vfs_dev *dev = zalloc(sizeof(*dev));
		strlcpy(dev->name, "hd", 3);
		dev->name[2] = 'a' + i;
		dev->type = DEV_BLOCK;
		dev->read = ata_read;
		dev->data = data;
		vfs_add_dev(dev);
		vfs_load(dev);
	}
}

CLEAR static void ata_find(u32 device, u16 vendor_id, u16 device_id, void *extra)
{
	if ((vendor_id == 0x8086) && (device_id == 0x7010))
		*((u32 *)extra) = device;
}

static u32 ata_device_pci = 0;
CLEAR void ata_install(void)
{
	pci_scan(&ata_find, -1, &ata_device_pci);
	assert(ata_device_pci);
	ide_buf = zalloc(SECTOR_SIZE);
	ata_probe();
}
