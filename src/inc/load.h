// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef LOAD_H
#define LOAD_H

#include <proc.h>

#define EFLAGS_ALWAYS 0x2 // Always one
#define EFLAGS_INTERRUPTS 0x200 // Enable interrupts

#define GDT_DATA_OFFSET 0x10 // Data segment offset in GDT
#define GDT_CODE_OFFSET 0x8 // Code segment offset in GDT

void bin_load(char *path, struct proc *proc);

#endif
