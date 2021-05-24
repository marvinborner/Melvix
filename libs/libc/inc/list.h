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
void list_destroy(struct list *list) NONNULL;
struct node *list_add(struct list *list, void *data) NONNULL;
struct list *list_remove(struct list *list, struct node *node) NONNULL;
struct node *list_last(struct list *list) NONNULL;
struct list *list_swap(struct list *list, struct node *a, struct node *b) NONNULL;
struct node *list_first_data(struct list *list, void *data) NONNULL;

#endif
