/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#ifndef PRINT_H
#define PRINT_H

#include <def.h>

NONNULL u32 log(const char *fmt, ...);
NONNULL void panic(const char *fmt, ...);

#endif
