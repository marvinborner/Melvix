/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#ifndef MANAGEMENT_TIMER_INDEX_H
#define MANAGEMENT_TIMER_INDEX_H

#include <sys.h>

struct timer {
	u32 start, diff;
	timer_callback_t callback;
	enum { TIMER_NONE, TIMER_TIMEOUT, TIMER_INTERVAL } type;
};

NONNULL void timer_add(struct timer *timer);
NONNULL void timer_iterate(void (*callback)(struct timer *, void *), void *data);

#endif
