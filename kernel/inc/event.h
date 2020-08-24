// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef EVENT_H
#define EVENT_H

#include <def.h>
#include <proc.h>
#include <sys.h>

struct event_descriptor {
	enum message_type id;
	struct proc *proc;
};

u32 event_register(enum message_type id, struct proc *proc);
void event_unregister(enum message_type id, struct proc *proc);
u32 event_trigger(enum message_type id, void *data);

#endif
