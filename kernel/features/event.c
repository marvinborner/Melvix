// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <def.h>
#include <sys.h>

u32 (*event_table[])() = { [EVENT_KEYBOARD] = NULL, [EVENT_MOUSE] = NULL };

u32 event_map(enum event id, u32 (*function)())
{
	// TODO: Support multiple events of same type
	if (id >= sizeof(event_table) / sizeof(*event_table))
		return -1;

	event_table[id] = function;
	return 0;
}

u32 event_trigger(enum event id, u32 *data)
{
	assert(id < sizeof(event_table) / sizeof(*event_table));

	u32 (*location)(u32 *) = event_table[id];
	if (!location) {
		printf("Event %d not mapped!\n", id);
		return -1;
	}

	// TODO: Execute event function in ring3 with process stack, ...
	location(data);
	return 0;
}
