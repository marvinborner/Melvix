// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <list.h>
#include <mem.h>

static int nonce = 0;

struct list *list_new()
{
	struct list *list = malloc(sizeof(*list));
	list->head = NULL;
	return list;
}

struct node *list_new_node()
{
	struct node *node = malloc(sizeof(*node));
	node->data = NULL;
	node->prev = NULL;
	node->next = NULL;
	node->nonce = nonce++;
	return node;
}

struct node *list_add_node(struct list *list, struct node *node)
{
	if (list == NULL)
		return NULL;

	if (list->head == NULL) {
		list->head = node;
		return list->head;
	}

	struct node *iterator = list->head;
	while (iterator != NULL) {
		if (iterator->next == NULL) {
			iterator->next = node;
			node->prev = iterator;
			break;
		}
		iterator = iterator->next;
	}
	return node;
}

struct node *list_add(struct list *list, void *data)
{
	struct node *node = list_new_node();
	node->data = data;
	return list_add_node(list, node);
}

// Maybe list_remove_node?
void list_remove(struct list *list, struct node *node)
{
	if (list == NULL || list->head == NULL)
		return;

	if (list->head == node) {
		list->head = list->head->next;
		return;
	}

	struct node *iterator = list->head->next;
	while (iterator != node) {
		iterator = iterator->next;
		if (iterator == NULL)
			return;
	}

	iterator->prev->next = iterator->next;
	iterator->next->prev = iterator->prev;
}
