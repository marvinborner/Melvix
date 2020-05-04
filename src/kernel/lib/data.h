#ifndef MELVIX_DATA_H
#define MELVIX_DATA_H

#include <stdint.h>

// LIST

struct list_node {
	struct list_node *prev;
	struct list_node *next;
	void *val;
};

struct list {
	struct list_node *head;
	struct list_node *tail;
	uint32_t size;
};

#define foreach(t, list) for (struct list_node *t = list->head; t != NULL; t = t->next)

struct list *list_create();

uint32_t list_size(struct list *list);

struct list_node *list_insert_front(struct list *list, void *val);

void list_insert_back(struct list *list, void *val);

void *list_remove_node(struct list *list, struct list_node *node);

void *list_remove_front(struct list *list);

void *list_remove_back(struct list *list);

void list_push(struct list *list, void *val);

struct list_node *list_pop(struct list *list);

void list_enqueue(struct list *list, void *val);

struct list_node *list_dequeue(struct list *list);

void *list_peek_front(struct list *list);

void *list_peek_back(struct list *list);

void list_destroy(struct list *list);

void listnode_destroy(struct list_node *node);

int list_contain(struct list *list, void *val);

struct list_node *list_get_node_by_index(struct list *list, int index);

void *list_remove_by_index(struct list *list, int index);

struct list *str_split(const char *str, const char *delim, uint32_t *numtokens);
char *list_to_str(struct list *list, const char *delim);

// Tree

struct tree_node {
	struct list *children;
	void *value;
};

struct tree {
	struct tree_node *root;
};

struct tree *tree_create();

struct tree_node *treenode_create(void *value);

struct tree_node *tree_insert(struct tree *tree, struct tree_node *subroot, void *value);

struct tree_node *tree_find_parent(struct tree *tree, struct tree_node *remove_node,
				   int *child_index);

struct tree_node *tree_find_parent_recur(struct tree *tree, struct tree_node *remove_node,
					 struct tree_node *subroot, int *child_index);

void tree_remove(struct tree *tree, struct tree_node *remove_node);

void tree2list_recur(struct tree_node *subroot, struct list *list);

void tree2list(struct tree *tree, struct list *list);

void tree2array(struct tree *tree, void **array, int *size);

void tree2array_recur(struct tree_node *subroot, void **array, int *size);

#endif