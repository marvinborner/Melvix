#include <stdint.h>
#include <stddef.h>
#include <common.h>
#include <system.h>

typedef u32 (*event_func)(u8 *);

u32 (*event_table[])() = { [MAP_KEYBOARD] = NULL, [MAP_MOUSE] = NULL };

u32 event_map(u32 id, u8 *function)
{
	if (id >= sizeof(event_map) / sizeof(*event_map))
		return -1;

	event_table[id] = (u32)function;
	return 0;
}

u32 event_trigger(u32 id, u8 *data)
{
	if (id >= sizeof(event_map) / sizeof(*event_map)) {
		panic("Unknown event id!");
		return -1;
	}

	event_func location = (event_func)event_table[id];
	if (!location) {
		warn("Event %d not mapped!", id);
		return -1;
	}

	location(data);
	return 0;
}