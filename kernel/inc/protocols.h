// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef PROTOCOLS_H
#define PROTOCOLS_H

#include <kernel.h>
#include <stdint.h>

struct boot_information *protocol_convert(u32 magic, uintptr_t address);

#endif
