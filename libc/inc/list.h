// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef LIST_H
#define LIST_H

#include <def.h>

struct list {
	struct node *head;
};

struct node {
	void *data;
	int nonce;
	struct node *next;
	struct node *prev;
};

struct list *list_new(void);
void list_destroy(struct list *list);
/* struct node *list_new_node(); */ // TODO: Make node-specific things static/private?
/* void list_add_node(struct list *list, struct node *node); */
struct node *list_add(struct list *list, void *data);
struct list *list_remove(struct list *list, struct node *node);
struct node *list_last(struct list *list);
struct list *list_swap(struct list *list, struct node *a, struct node *b);
struct node *list_first_data(struct list *list, void *data);

#endif
