// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef CORE_FLOAT_H
#define CORE_FLOAT_H

#include <def.h>

// TODO!
struct proc {
	u8 fpu[512];
};

void fpu_enable(void);
NONNULL void fpu_init(struct proc *proc);
NONNULL void fpu_save(struct proc *proc);
NONNULL void fpu_restore(struct proc *proc);

#endif
