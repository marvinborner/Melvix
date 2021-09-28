// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef BOOT_ABSTRACT_H
#define BOOT_ABSTRACT_H

#include <def.h>

extern int kernel_main(void);

NONNULL void abstract_boot_memory_map_set(void *memory_map, u32 memory_map_length);
NONNULL void abstract_boot_arguments_set(char *arguments);
NONNULL void abstract_boot_vbe_info_set(void *vbe_mode_info);

void abstract_boot_finish(void);

#endif
