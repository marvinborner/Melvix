// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef X86_32_INTERRUPTS_PIC_H
#define X86_32_INTERRUPTS_PIC_H

#include <kernel.h>

void pic_init(void);
void pic_ack(u32 int_no);

#endif
