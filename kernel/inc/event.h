// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef EVENT_H
#define EVENT_H

#include <def.h>
#include <proc.h>
#include <sys.h>

struct event_descriptor {
	u32 id;
	struct proc *proc;
};

u32 event_register(u32 id, struct proc *proc);
void event_unregister(u32 id, struct proc *proc);
u32 event_trigger(u32 id, void *data);

#endif