// MIT License, Copyright (c) 2020 Marvin Borner

#include <cpu.h>
#include <def.h>
#include <ide.h>
#include <print.h>

int ide_stat()
{
	inb(IDE_IO + IDE_CMD);
	inb(IDE_IO + IDE_CMD);
	inb(IDE_IO + IDE_CMD);
	inb(IDE_IO + IDE_CMD);
	return inb(IDE_IO + IDE_CMD);
}

void ide_wait()
{
	int stat = 0;
	do
		stat = ide_stat();
	while ((stat & IDE_BUSY) != 0);
}

// TODO: Fix strange ide_read bugs
void *ide_read(void *b, u32 block)
{
	u8 sector_count = BLOCK_SIZE / SECTOR_SIZE; // 2
	u32 sector = block * sector_count;

	outb(IDE_IO + IDE_SECTOR_COUNT, (u8)sector_count); // Number of sectors

	outb(IDE_IO + IDE_LOW, LBA_LOW(sector));
	outb(IDE_IO + IDE_MID, LBA_MID(sector));
	outb(IDE_IO + IDE_HIGH, LBA_HIGH(sector));

	// Slave/Master << 4 and last 4 bits
	outb(IDE_IO + IDE_SELECT, 0xE0 | (1 << 4) | LBA_LAST(sector));

	outb(IDE_IO + IDE_CMD, IDE_CMD_READ);

	ide_wait();
	insl(IDE_IO, b, BLOCK_SIZE / 4);

	return b;
}
