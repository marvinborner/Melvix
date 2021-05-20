// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef VMWARE_H
#define VMWARE_H

#include <def.h>

u8 vmware_detect(void);
u8 vmware_mouse_detect(void);
void vmware_mouse_install(u8 device);

#endif
