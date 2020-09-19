// MIT License, Copyright (c) 2020 Marvin Borner

#include <cpu.h>
#include <def.h>
#include <ide.h>

int ide_wait(int check)
{
	char r;

	// Wait while drive is busy. Once just ready is set, exit the loop
	while (((r = (char)inb(IDE_IO | IDE_CMD)) & (IDE_BUSY | IDE_READY)) != IDE_READY)
		;

	// Check for errors
	if (check && (r & (IDE_DRIVE_FAULT | IDE_ERROR)) != 0)
		return 0xF;
	return 0;
}

void __attribute__((optimize("O0"))) * ide_read(void *b, u32 block)
{
	int sector_per_block = BLOCK_SIZE / SECTOR_SIZE; // 2
	int sector = block * sector_per_block;

	ide_wait(0);
	outb(IDE_IO | IDE_SECTOR_COUNT, sector_per_block); // Number of sectors
	outb(IDE_IO | IDE_LOW, LBA_LOW(sector));
	outb(IDE_IO | IDE_MID, LBA_MID(sector));
	outb(IDE_IO | IDE_HIGH, LBA_HIGH(sector));

	// Slave/Master << 4 and last 4 bits
	outb(IDE_IO | IDE_HEAD, 0xE0 | (1 << 4) | LBA_LAST(sector));
	outb(IDE_IO | IDE_CMD, IDE_CMD_READ);
	ide_wait(0);

	// Read-only
	insl(IDE_IO, b, BLOCK_SIZE / 4);

	return b;
}
