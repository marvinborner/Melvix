// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef CORE_IO_H
#define CORE_IO_H

#include <def.h>

u8 inb(u16 port);
u16 inw(u16 port);
u32 inl(u16 port);
void outb(u16 port, u8 data);
void outw(u16 port, u16 data);
void outl(u16 port, u32 data);

#endif
