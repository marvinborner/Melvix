// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <cpu.h>
#include <def.h>
#include <fs.h>
#include <ide.h>
#include <mem.h>
#include <print.h>

static u8 *ide_buf = NULL;

struct ata_data {
	u8 drive;
};

void ide_select_drive(u8 bus, u8 drive)
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

u8 ide_find(u8 bus, u8 drive)
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

void ide_delay(u16 io) // 400ns
{
	for (int i = 0; i < 4; i++)
		inb(io + ATA_REG_ALTSTATUS);
}

void ide_poll(u16 io)
{
	for (int i = 0; i < 4; i++)
		inb(io + ATA_REG_ALTSTATUS);

	u8 status;
	do {
		status = inb(io + ATA_REG_STATUS);
	} while (status & ATA_SR_BSY);

	do {
		status = inb(io + ATA_REG_STATUS);
		assert(!(status & ATA_SR_ERR))
	} while (!(status & ATA_SR_DRQ));
}

u8 ata_read_one(u8 *buf, u32 lba, struct device *dev)
{
	u8 drive = ((struct ata_data *)dev->data)->drive;
	u16 io = (drive & ATA_PRIMARY << 1) == ATA_PRIMARY ? ATA_PRIMARY_IO : ATA_SECONDARY_IO;
	drive = (drive & ATA_SLAVE) == ATA_SLAVE ? ATA_SLAVE : ATA_MASTER;
	u8 cmd = drive == ATA_MASTER ? 0xe0 : 0xf0;
	outb(io + ATA_REG_HDDEVSEL, (cmd | (u8)((lba >> 24 & 0x0f))));
	outb(io + 1, 0x00);
	outb(io + ATA_REG_SECCOUNT0, 1);
	outb(io + ATA_REG_LBA0, (u8)lba);
	outb(io + ATA_REG_LBA1, (u8)(lba >> 8));
	outb(io + ATA_REG_LBA2, (u8)(lba >> 16));
	outb(io + ATA_REG_COMMAND, ATA_CMD_READ_PIO);
	ide_poll(io);

	for (int i = 0; i < BLOCK_COUNT; i++) {
		u16 data = inw(io + ATA_REG_DATA);
		*(u16 *)(buf + i * 2) = data;
	}
	ide_delay(io);
	return 1;
}

s32 ata_read(void *buf, u32 lba, u32 sector_count, struct device *dev)
{
	u8 *b = buf; // I love bytes, yk
	for (u32 i = 0; i < sector_count; i++) {
		ata_read_one(b, lba + i, dev);
		b += SECTOR_SIZE;
	}
	return sector_count;
}

int ata_pm = 0, ata_ps = 0, ata_sm = 0, ata_ss = 0;
void ata_probe(void)
{
	for (int i = 0; i < 4; i++) {
		int bus = i < 2 ? ATA_PRIMARY : ATA_SECONDARY;
		int drive = i % 2 ? ATA_MASTER : ATA_SLAVE;

		if (!ide_find(bus, drive))
			continue;

		struct device *dev = malloc(sizeof(*dev));
		struct ata_data *data = malloc(sizeof(*data));
		data->drive = (bus << 1) | drive;

		char *str = malloc(40);
		for (int j = 0; j < 40; j += 2) {
			str[j] = ide_buf[ATA_IDENT_MODEL + j + 1];
			str[j + 1] = ide_buf[ATA_IDENT_MODEL + j];
		}

		dev->name = str;
		dev->type = DEV_BLOCK;
		dev->read = ata_read;
		device_add(dev);
		if (vfs_mounted(dev, "/"))
			continue;

		// TODO: Check if ext2 first
		struct vfs *vfs = malloc(sizeof(*vfs));
		vfs->type = VFS_EXT2;
		vfs->read = ext2_read;
		vfs->ready = ext2_ready;
		vfs->stat = ext2_stat;
		dev->vfs = vfs;
		dev->data = data;
		vfs_mount(dev, "/");
	}
}

void ata_install(void)
{
	ide_buf = malloc(SECTOR_SIZE);
	ata_probe();
}
