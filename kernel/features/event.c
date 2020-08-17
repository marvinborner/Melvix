// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <def.h>
#include <list.h>
#include <sys.h>

struct list *event_table[] = { [EVENT_KEYBOARD] = NULL, [EVENT_MOUSE] = NULL };

u32 event_map(enum event id, u32 *func)
{
	// TODO: Check if function is already mapped
	if (id >= sizeof(event_table) / sizeof(*event_table))
		return -1;

	if (event_table[id] == NULL)
		event_table[id] = (struct list *)list_new();
	list_add((struct list *)event_table[id], (void *)func);
	return 0;
}

// TODO: Fix unmap
void event_unmap(enum event id, u32 *func)
{
	struct list *list = ((struct list *)event_table[id]);
	struct node *iterator = list->head;
	while (iterator->data != (void *)func) {
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

	if (!iterator->data) {
		printf("Event %d not mapped!\n", id);
		return 1;
	}

	while (1) {
		u32 *func = iterator->data;
		iterator = iterator->next;
		if (iterator == NULL)
			break;
	}

	// TODO: Execute event function in ring3 with process stack, ...
	/* location(data); */
	return 0;
}
