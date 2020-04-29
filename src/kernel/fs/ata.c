#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <kernel/fs/ata.h>
#include <kernel/system.h>
#include <kernel/io/io.h>

static uint16_t sel_base_port = 0;
static uint8_t sel_master_or_slave = 0;

static uint32_t max_sector;

static uint8_t read_stat(uint16_t base)
{
	inb(base + COM_STAT);
	inb(base + COM_STAT);
	inb(base + COM_STAT);
	inb(base + COM_STAT);

	return inb(base + COM_STAT);
}

static void check_drive(uint16_t base, uint8_t master_or_slave)
{
	if (sel_base_port != 0)
		return;

	outb(base + DRIVE_SELECT, master_or_slave);

	outb(base + SECTOR_COUNT, 0);
	outb(base + LBA_LOW, 0);
	outb(base + LBA_MID, 0);
	outb(base + LBA_HIGH, 0);

	outb(base + COM_STAT, IDENTIFY);
	uint8_t stat = read_stat(base);

	if (stat == 0)
		return;

	while ((stat & BSY) != 0)
		stat = read_stat(base);

	while ((stat & DRQ) == 0 && (stat & ERR) == 0)
		stat = read_stat(base);

	if ((stat & ERR) != 0)
		return;

	uint16_t drive_data[256];
	for (size_t i = 0; i < 256; i++)
		drive_data[i] = inw(base + DATA);

	max_sector = drive_data[MAX_28LBA_SECTORS] | drive_data[MAX_28LBA_SECTORS + 1] << 16;

	sel_base_port = base;
	sel_master_or_slave = master_or_slave;
}

void ata_init()
{
	uint8_t pri_status = inb(PRIMARY_BASE + COM_STAT);
	uint8_t sec_status = inb(SECONDARY_BASE + COM_STAT);
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
	uint8_t stat;

	do
		stat = read_stat(sel_base_port);
	while ((stat & BSY) != 0);
}

void read_abs_sectors(uint32_t lba, uint8_t sector_count, uint16_t buf[])
{
	assert(lba >> LBA_BITS == 0);

	outb(sel_base_port + DRIVE_SELECT, (lba >> (LBA_BITS - 4)) | sel_master_or_slave | 1 << 6);
	outb(sel_base_port + SECTOR_COUNT, sector_count);

	outb(sel_base_port + LBA_LOW, lba & 0xFF);
	outb(sel_base_port + LBA_MID, (lba >> 8) & 0xFF);
	outb(sel_base_port + LBA_HIGH, (lba >> 16) & 0xFF);

	outb(sel_base_port + COM_STAT, READ_SECTORS);

	size_t i = 0;
	for (; sector_count > 0; sector_count--) {
		poll();

		asm("rep insw" ::"c"(SECTOR_SIZE / 2), "d"(sel_base_port + DATA), "D"(buf + i));
		i += SECTOR_SIZE / 2;
	}
}