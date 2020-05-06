#ifndef MELVIX_EVENT_H
#define MELVIX_EVENT_H

#include <stdint.h>

u32 event_map(u32 id, u8 *function);
u32 event_trigger(u32 id, u8 *data);

#endif