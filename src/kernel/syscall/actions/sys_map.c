#include <events/event.h>
#include <stdint.h>

u32 sys_map(u32 id, u8 *function)
{
	event_map(id, function);
	return -1;
}