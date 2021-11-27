/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#ifndef MANAGEMENT_INTERRUPT_INDEX_H
#define MANAGEMENT_INTERRUPT_INDEX_H

#include <def.h>
#include <sys.h>

NONNULL void interrupt_add(u32 interrupt, interrupt_t handler);
interrupt_t interrupt_get(u32 interrupt);

#endif
