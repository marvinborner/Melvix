/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#ifndef EVENTS_MOUSE_H
#define EVENTS_MOUSE_H

#include <err.h>

struct mouse_data {
	u32 x, y;
	s32 wheel;
	struct {
		u8 left : 1;
		u8 right : 1;
	} button;
};

NONNULL err mouse_handler(struct mouse_data *data);

#endif
