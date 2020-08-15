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

struct list *list_new();
/* struct node *list_new_node(); */ // TODO: Make node-specific things static/private?
/* void list_add_node(struct list *list, struct node *node); */
void list_add(struct list *list, void *data);
void list_remove(struct list *list, struct node *node);

#endif
