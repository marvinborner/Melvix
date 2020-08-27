// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef IDE_H
#define IDE_H

#include <def.h>

#define BLOCK_SIZE 1024
#define BLOCK_COUNT 256 // BLOCK_SIZE / sizeof(u32)
#define SECTOR_SIZE 512

#define IDE_BUSY (1 << 7)
#define IDE_READY (1 << 6)
#define IDE_DRIVE_FAULT (1 << 5)
#define IDE_ERROR (1 << 0)

#define IDE_IO 0x1F0
#define IDE_DATA 0x0
#define IDE_FEATURES 0x1
#define IDE_SECTOR_COUNT 0x2
#define IDE_LOW 0x3
#define IDE_MID 0x4
#define IDE_HIGH 0x5
#define IDE_HEAD 0x6
#define IDE_CMD 0x7
#define IDE_ALTERNATE 0x3F6

#define LBA_LOW(c) ((u8)(c & 0xFF))
#define LBA_MID(c) ((u8)(c >> 8) & 0xFF)
#define LBA_HIGH(c) ((u8)(c >> 16) & 0xFF)
#define LBA_LAST(c) ((u8)(c >> 24) & 0xF)

#define IDE_CMD_READ (BLOCK_SIZE / SECTOR_SIZE == 1) ? 0x20 : 0xC4
#define IDE_CMD_WRITE (BLOCK_SIZE / SECTOR_SIZE == 1) ? 0x30 : 0xC5
#define IDE_CMD_READ_MUL 0xC4
#define IDE_CMD_WRITE_MUL 0xC5

int ide_wait(int check);
void *ide_read(void *b, u32 block);

#endif
