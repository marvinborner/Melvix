// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <def.h>
#include <event.h>
#include <list.h>
#include <mem.h>
#include <proc.h>
#include <sys.h>

struct list *event_table[] = { [EVENT_KEYBOARD] = NULL, [EVENT_MOUSE] = NULL };

u32 event_map(enum event id, u32 *func)
{
	// TODO: Check if function is already mapped
	if (id >= sizeof(event_table) / sizeof(*event_table))
		return -1;

	if (event_table[id] == NULL)
		event_table[id] = (struct list *)list_new();

	struct event_descriptor *desc = malloc(sizeof(*desc));
	desc->func = func;
	desc->proc = proc_current();

	list_add((struct list *)event_table[id], (void *)desc);
	return 0;
}

// TODO: Fix unmap
void event_unmap(enum event id, u32 *func)
{
	struct list *list = ((struct list *)event_table[id]);

	struct event_descriptor *desc = malloc(sizeof(*desc));
	desc->func = func;
	desc->proc = proc_current();

	struct node *iterator = list->head;
	while (memcmp(iterator->data, (void *)desc, sizeof(*desc)) == 0) {
		iterator = iterator->next;
		if (!iterator)
			return;
	}

	list_remove(list, iterator);
}

u32 event_trigger(enum event id, u32 *data)
{
	assert(id < sizeof(event_table) / sizeof(*event_table));

	struct node *iterator = ((struct list *)event_table[id])->head;

	if (memcmp(event_table[id], 0, sizeof(struct list)) == 0) {
		printf("Event %d not mapped!\n", id);
		return 1;
	}

	while (1) {
		struct event_descriptor *desc = iterator->data;
		desc->proc->event = 1 << id;
		iterator = iterator->next;
		if (iterator == NULL)
			break;
	}

	return 0;
}
