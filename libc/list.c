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
	if (!list || !node)
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

struct node *list_last(struct list *list)
{
	if (!list || !list->head)
		return NULL;

	struct node *iterator = list->head;
	while (iterator != NULL) {
		if (iterator->next == NULL)
			return iterator;
		iterator = iterator->next;
	}

	return NULL;
}

struct node *list_first_data(struct list *list, void *data)
{
	if (!list || !list->head || !data)
		return NULL;

	struct node *iterator = list->head;
	while (iterator != NULL) {
		if (iterator->data == data)
			return iterator;
		iterator = iterator->next;
	}

	return NULL;
}

// TODO: Actually swap the nodes, not the data
struct list *list_swap(struct list *list, struct node *a, struct node *b)
{
	if (!list || !list->head || !a || !b)
		return NULL;

	void *tmp = a->data;
	a->data = b->data;
	b->data = tmp;

	return list;
}

struct node *list_add(struct list *list, void *data)
{
	struct node *node = list_new_node();
	node->data = data;
	return list_add_node(list, node);
}

// Maybe list_remove_node?
struct list *list_remove(struct list *list, struct node *node)
{
	if (!list || !list->head || !node)
		return NULL;

	if (list->head == node) {
		list->head = list->head->next;
		return list;
	}

	struct node *iterator = list->head->next;
	while (iterator != node) {
		iterator = iterator->next;
		if (iterator == NULL)
			return NULL;
	}

	iterator->prev->next = iterator->next;
	iterator->next->prev = iterator->prev;
	return list;
}
