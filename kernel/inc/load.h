// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef LOAD_H
#define LOAD_H

#include <proc.h>

void proc_load(struct proc *proc, void *data);
int bin_load(const char *path, struct proc *proc);

#endif
