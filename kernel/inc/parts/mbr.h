/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#ifndef PARTS_MBR_H
#define PARTS_MBR_H

#include <err.h>

err mbr_load(u32 disk);

#endif
