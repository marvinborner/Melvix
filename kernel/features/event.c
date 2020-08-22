// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <def.h>
#include <event.h>
#include <list.h>
#include <mem.h>
#include <proc.h>
#include <sys.h>

struct list *event_table[] = { [EVENT_KEYBOARD] = NULL, [EVENT_MOUSE] = NULL };

u32 event_map(enum event id, struct proc *proc, u32 *func)
{
	assert(id < sizeof(event_table) / sizeof(*event_table));
	assert(func);

	if (event_table[id] == NULL)
		event_table[id] = (struct list *)list_new();

	struct node *iterator = event_table[id]->head;
	do {
		assert(((struct event_descriptor *)iterator->data)->func != func);
	} while ((iterator = iterator->next) != NULL);

	struct event_descriptor *desc = malloc(sizeof(*desc));
	desc->id = id;
	desc->func = func;
	desc->proc = proc;

	list_add(event_table[id], (void *)desc);
	return 0;
}

void event_unmap(enum event id, struct proc *proc, u32 *func)
{
	assert(id < sizeof(event_table) / sizeof(*event_table));
	assert(func);

	struct list *list = event_table[id];

	struct event_descriptor *desc = malloc(sizeof(*desc));
	desc->id = id;
	desc->func = func;
	desc->proc = proc;

	struct node *iterator = list->head;
	do {
		if (memcmp(iterator->data, desc, sizeof(*desc)) == 0)
			list_remove(list, iterator);
	} while ((iterator = iterator->next) != NULL);
}

u32 event_trigger(enum event id, void *data)
{
	(void)data;
	assert(id < sizeof(event_table) / sizeof(*event_table));

	struct node *iterator = event_table[id]->head;

	if (memcmp(event_table[id], 0, sizeof(struct list)) == 0 || !event_table[id]->head) {
		printf("Event %d not mapped!\n", id);
		return 1;
	}

	while (1) {
		struct proc_event *proc_event = malloc(sizeof(*proc_event));
		struct event_descriptor *desc = iterator->data;
		proc_event->desc = desc;
		proc_event->data = data;
		list_add(desc->proc->events, proc_event);
		iterator = iterator->next;
		if (iterator == NULL)
			break;
	}

	return 0;
}
