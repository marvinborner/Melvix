// MIT License, Copyright (c) 2020 Marvin Borner
// This file specifies the structs passed by the bootloader

#ifndef BOOT_H
#define BOOT_H

#include <def.h>

extern struct vid_info *boot_passed;
struct vid_info {
	u32 mode;
	u32 *vbe;
};

#endif
