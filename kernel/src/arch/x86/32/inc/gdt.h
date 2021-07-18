// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef GDT_H
#define GDT_H

#define GDT_CODE_SEGMENT 0x08
#define GDT_DATA_SEGMENT 0x10

void gdt_init(void);

#endif
