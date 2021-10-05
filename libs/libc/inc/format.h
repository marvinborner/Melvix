/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#ifndef FORMAT_H
#define FORMAT_H

#include <arg.h>
#include <def.h>

NONNULL u32 format(char *out, u32 size, const char *format, va_list ap);

#endif
