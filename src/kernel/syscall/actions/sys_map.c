#include <stdint.h>
#include <events/event.h>

u32 sys_map(u32 id, u32 *function)
{
	event_map(id, function);
	return -1;
}