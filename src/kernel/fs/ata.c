#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <fs/ata.h>
#include <system.h>
#include <io/io.h>

static u16 sel_base_port = 0;
static u8 sel_master_or_slave = 0;

static u32 max_sector;

static u8 read_stat(u16 base)
{
	inb(base + COM_STAT);
	inb(base + COM_STAT);
	inb(base + COM_STAT);
	inb(base + COM_STAT);

	return inb(base + COM_STAT);
}

static void check_drive(u16 base, u8 master_or_slave)
{
	if (sel_base_port != 0)
		return;

	outb(base + DRIVE_SELECT, master_or_slave);

	outb(base + SECTOR_COUNT, 0);
	outb(base + LBA_LOW, 0);
	outb(base + LBA_MID, 0);
	outb(base + LBA_HIGH, 0);

	outb(base + COM_STAT, IDENTIFY);
	u8 stat = read_stat(base);

	if (stat == 0)
		return;

	while ((stat & BSY) != 0)
		stat = read_stat(base);

	while ((stat & DRQ) == 0 && (stat & ERR) == 0)
		stat = read_stat(base);

	if ((stat & ERR) != 0)
		return;

	u16 drive_data[256];
	for (u32 i = 0; i < 256; i++)
		drive_data[i] = inw(base + DATA);

	max_sector = drive_data[MAX_28LBA_SECTORS] | drive_data[MAX_28LBA_SECTORS + 1] << 16;

	sel_base_port = base;
	sel_master_or_slave = master_or_slave;
}

void ata_init()
{
	u8 pri_status = inb(PRIMARY_BASE + COM_STAT);
	u8 sec_status = inb(SECONDARY_BASE + COM_STAT);
	bool primary_floating = false;
	bool secondary_floating = false;
	if (pri_status == 0xFF)
		primary_floating = true;
	if (sec_status == 0xFF)
		secondary_floating = true;

	if (primary_floating && secondary_floating) {
		log("No drives attached! What's going on?");
		return;
	}

	check_drive(PRIMARY_BASE, SEL_MASTER);
	check_drive(PRIMARY_BASE, SEL_SLAVE);
	check_drive(SECONDARY_BASE, SEL_MASTER);
	check_drive(SECONDARY_BASE, SEL_SLAVE);

	if (sel_base_port == 0)
		log("No drives attached! What's going on?");
	else {
		log("Found drive: Selecting %s on the %s bus",
		    sel_master_or_slave == SEL_MASTER ? "master" : "slave",
		    sel_base_port == PRIMARY_BASE ? "primary" : "secondary");
		log("Max LBA value is %d", max_sector);
	}
}

static void poll()
{
	u8 stat;

	do
		stat = read_stat(sel_base_port);
	while ((stat & BSY) != 0);
}

void read_abs_sectors(u32 lba, u8 sector_count, u16 buf[])
{
	assert(lba >> LBA_BITS == 0);

	outb(sel_base_port + DRIVE_SELECT, (lba >> (LBA_BITS - 4)) | sel_master_or_slave | 1 << 6);
	outb(sel_base_port + SECTOR_COUNT, sector_count);

	outb(sel_base_port + LBA_LOW, lba & 0xFF);
	outb(sel_base_port + LBA_MID, (lba >> 8) & 0xFF);
	outb(sel_base_port + LBA_HIGH, (lba >> 16) & 0xFF);

	outb(sel_base_port + COM_STAT, READ_SECTORS);

	u32 i = 0;
	for (; sector_count > 0; sector_count--) {
		poll();

		asm("rep insw" ::"c"(SECTOR_SIZE / 2), "d"(sel_base_port + DATA), "D"(buf + i));
		i += SECTOR_SIZE / 2;
	}
}