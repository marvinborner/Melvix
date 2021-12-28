/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <print.h>

#include <drivers/interrupt.h>
#include <events/interrupt.h>

struct interrupt_data *interrupt_handler(struct interrupt_data *frame)
{
	u32 effective = frame->interrupt - 32;

	err call = interrupt_call(effective, frame);
	if (call != ERR_OK)
		log("Interrupt %d failed: %e", effective, call);
	err request = device_request(DEVICE_INTERRUPT, DEVICE_INTERRUPT_ACK, frame->interrupt);
	if (request != ERR_OK)
		log("Interrupt %d ack failed for: %e", effective, request);
	return frame;
}
