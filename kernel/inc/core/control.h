// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef CORE_CONTROL_H
#define CORE_CONTROL_H

#include <def.h>

u32 cr0_get(void);
void cr0_set(u32 cr0);
u32 cr3_get(void);
void cr3_set(u32 cr3);
u32 cr4_get(void);
void cr4_set(u32 cr4);

#endif
