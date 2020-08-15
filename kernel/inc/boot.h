// MIT License, Copyright (c) 2020 Marvin Borner
// This file specifies the structs passed by the bootloader

#include <def.h>

struct vid_info *boot_passed;
struct vid_info {
	u32 mode;
	u32 *vbe;
};
