/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <print.h>
#include <sys.h>

#include <drivers/timer.h>
#include <management/timer/index.h>

static void looper(struct timer *timer, void *data)
{
	u32 time = (u32)data;

	if (timer->type == TIMER_TIMEOUT) {
		if (time - timer->start >= timer->diff) {
			void (*callback)(u32) = timer->callback;
			timer->callback = 0;
			callback(time); // TODO: Callback using better time diff
		}
	} else if (timer->type == TIMER_INTERVAL) {
		if (time - timer->start >= timer->diff) {
			timer->start = time;
			timer->callback(time);
		}
	} else {
		log("Invalid timer type %d", timer->type);
	}
}

err timer_execute(u32 time)
{
	timer_iterate(&looper, (void *)time);
	return ERR_OK;
}

err timer_interval(u32 interval, timer_callback_t callback)
{
	u32 time = device_request(DEVICE_TIMER, DEVICE_TIMER_GET);
	struct timer timer = {
		.start = time,
		.diff = interval,
		.callback = callback,
		.type = TIMER_INTERVAL,
	};
	timer_add(&timer);
	return ERR_OK;
}

err timer_timeout(u32 timeout, timer_callback_t callback)
{
	u32 time = device_request(DEVICE_TIMER, DEVICE_TIMER_GET);
	struct timer timer = {
		.start = time,
		.diff = timeout,
		.callback = callback,
		.type = TIMER_TIMEOUT,
	};
	timer_add(&timer);
	return ERR_OK;
}
