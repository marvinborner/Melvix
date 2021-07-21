// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef X86_SERIAL_H
#define X86_SERIAL_H

#include <kernel.h>

void serial_enable(void);
void serial_disable(void);

u8 serial_init(void);
void serial_print(const char *data, size_t count) NONNULL;

#endif
