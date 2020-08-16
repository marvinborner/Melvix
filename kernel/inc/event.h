// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef EVENT_H
#define EVENT_H

#include <def.h>
#include <sys.h>

u32 event_map(enum event id, u32 (*function)());
u32 event_trigger(enum event id, u32 *data);

#endif
