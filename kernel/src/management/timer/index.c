/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <print.h>

#include <management/timer/index.h>

PROTECTED static struct timer timers[512] = { 0 };

void timer_add(struct timer *timer)
{
	if (!timer->callback) {
		log("Timer callback missing");
		return;
	}

	if (timer->type == TIMER_NONE) {
		log("Timer type missing");
		return;
	}

	for (u32 i = 0; i < COUNT(timers); i++) {
		if (!timers[i].callback) {
			timers[i] = *timer;
			return;
		}
	}

	log("Timer slot could not be found");
}

void timer_iterate(void (*callback)(struct timer *, void *), void *data)
{
	for (u32 i = 0; i < COUNT(timers); i++)
		if (timers[i].callback)
			callback(&timers[i], data);
}
