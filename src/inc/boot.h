// MIT License, Copyright (c) 2020 Marvin Borner
// This file specifies the structs passed by the bootloader

#include <def.h>
#include <vesa.h>

struct vid_info {
	u32 mode;
	struct vbe *info;
};
