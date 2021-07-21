// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef X86_CPU_H
#define X86_CPU_H

#include <kernel.h>

u8 inb(u16 port);
u16 inw(u16 port);
u32 inl(u16 port);
void outb(u16 port, u8 data);
void outw(u16 port, u16 data);
void outl(u16 port, u32 data);

u32 cr0_get(void);
void cr0_set(u32 cr0);

u32 cr3_get(void);
void cr3_set(u32 cr3);

u32 cr4_get(void);
void cr4_set(u32 cr4);

#endif
