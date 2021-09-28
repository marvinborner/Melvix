// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef CORE_DESCRIPTORS_GLOBAL_H
#define CORE_DESCRIPTORS_GLOBAL_H

#include <def.h>

void tss_stack_set(u32 esp);
void gdt_init(void);

#endif
