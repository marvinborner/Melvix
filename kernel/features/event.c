// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <def.h>
#include <event.h>
#include <list.h>
#include <mem.h>
#include <proc.h>
#include <sys.h>

struct list *event_table[] = { [EVENT_KEYBOARD] = NULL, [EVENT_MOUSE] = NULL };

u32 event_register(u32 id, struct proc *proc)
{
	assert(id < sizeof(event_table) / sizeof(*event_table));

	if (event_table[id] == NULL)
		event_table[id] = (struct list *)list_new();

	struct event_descriptor *desc = malloc(sizeof(*desc));
	desc->id = id;
	desc->proc = proc;

	list_add(event_table[id], (void *)desc);
	return 0;
}

void event_unregister(u32 id, struct proc *proc)
{
	assert(id < sizeof(event_table) / sizeof(*event_table));

	struct event_descriptor desc;
	desc.id = id;
	desc.proc = proc;

	struct node *iterator = event_table[id]->head;
	while (iterator != NULL) {
		struct event_descriptor *desc_comp = iterator->data;
		if (desc_comp->id == desc.id && desc_comp->proc == desc.proc)
			list_remove(event_table[id], iterator);
		iterator = iterator->next;
	}
}

u32 event_trigger(u32 id, void *data)
{
	assert(id < sizeof(event_table) / sizeof(*event_table));

	if (memcmp(event_table[id], 0, sizeof(struct list)) == 0 || !event_table[id]->head) {
		printf("Event %d not mapped!\n", id);
		return 1;
	}

	struct node *iterator = event_table[id]->head;
	while (iterator != NULL) {
		proc_send(kernel_proc, ((struct event_descriptor *)iterator->data)->proc, id, data);
		iterator = iterator->next;
	}

	return 0;
}
