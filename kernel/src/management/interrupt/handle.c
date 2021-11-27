/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <print.h>
#include <sys.h>

#include <drivers/interrupt.h>
#include <management/interrupt/handle.h>

void *interrupt_handler(void *data)
{
	struct interrupt_frame *frame = data;
	u32 effective = frame->interrupt - 32;

	err call = interrupt_call(effective, data);
	if (call != ERR_OK)
		log("Interrupt %x failed with %e", effective, call);
	err request = device_request(DEVICE_INTERRUPT, DEVICE_INTERRUPT_ACK);
	if (request != ERR_OK)
		log("Interrupt ack failed with %e", call);
	return data;
}
