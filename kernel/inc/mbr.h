// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef MBR_H
#define MBR_H

#include <def.h>
#include <fs.h>

struct mbr_entry {
	u8 attributes;
	u8 chs_start[3];
	u8 type;
	u8 chs_end[3];
	u32 start;
	u32 size;
} PACKED;

struct mbr {
	u8 bootstrap[440];
	u32 signature;
	u16 reserved;
	struct mbr_entry entries[4];
	u16 magic;
} PACKED;

u8 mbr_load(struct vfs_dev *dev);

#endif
