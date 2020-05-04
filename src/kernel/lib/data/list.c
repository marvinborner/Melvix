#include <stdint.h>
#include <stddef.h>
#include <kernel/lib/lib.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/data.h>
#include <kernel/memory/alloc.h>

struct list *list_create()
{
	struct list *list = kcalloc(sizeof(struct list), 1);
	return list;
}

uint32_t list_size(struct list *list)
{
	if (!list)
		return 0;
	return list->size;
}

void *list_remove_node(struct list *list, struct list_node *node)
{
	void *val = node->val;
	if (list->head == node)
		return list_remove_front(list);
	else if (list->tail == node)
		return list_remove_back(list);
	else {
		node->next->prev = node->prev;
		node->prev->next = node->next;
		list->size--;
		kfree(node);
	}
	return val;
}
struct list_node *list_insert_front(struct list *list, void *val)
{
	struct list_node *t = kcalloc(sizeof(struct list_node), 1);
	list->head->prev = t;
	t->next = list->head;
	t->val = val;

	if (!list->head)
		list->tail = t;

	list->head = t;
	list->size++;
	return t;
}

void list_insert_back(struct list *list, void *val)
{
	struct list_node *t = kcalloc(sizeof(struct list_node), 1);
	t->prev = list->tail;
	if (list->tail)
		list->tail->next = t;
	t->val = val;

	if (!list->head)
		list->head = t;

	list->tail = t;
	list->size++;
}

void *list_remove_front(struct list *list)
{
	if (!list->head)
		return NULL;
	struct list_node *t = list->head;
	void *val = t->val;
	list->head = t->next;
	if (list->head)
		list->head->prev = NULL;
	kfree(t);
	list->size--;
	return val;
}

void *list_remove_back(struct list *list)
{
	if (!list->head)
		return NULL;
	struct list_node *t = list->tail;
	void *val = t->val;
	list->tail = t->prev;
	if (list->tail)
		list->tail->next = NULL;
	kfree(t);
	list->size--;
	return val;
}

void list_push(struct list *list, void *val)
{
	list_insert_back(list, val);
}

struct list_node *list_pop(struct list *list)
{
	if (!list->head)
		return NULL;
	struct list_node *t = list->tail;
	list->tail = t->prev;
	if (list->tail)
		list->tail->next = NULL;
	list->size--;
	return t;
}

void list_enqueue(struct list *list, void *val)
{
	list_insert_front(list, val);
}

struct list_node *list_dequeue(struct list *list)
{
	return list_pop(list);
}

void *list_peek_front(struct list *list)
{
	if (!list->head)
		return NULL;
	return list->head->val;
}

void *list_peek_back(struct list *list)
{
	if (!list->tail)
		return NULL;
	return list->tail->val;
}

int list_contain(struct list *list, void *val)
{
	int idx = 0;
	foreach(listnode, list)
	{
		if (listnode->val == val)
			return idx;
		idx++;
	}
	return -1;
}

struct list_node *list_get_node_by_index(struct list *list, int index)
{
	if (index < 0 || index >= list_size(list))
		return NULL;
	int curr = 0;
	foreach(listnode, list)
	{
		if (index == curr)
			return listnode;
		curr++;
	}
	return NULL;
}

void *list_remove_by_index(struct list *list, int index)
{
	struct list_node *node = list_get_node_by_index(list, index);
	return list_remove_node(list, node);
}

void list_destroy(struct list *list)
{
	struct list_node *node = list->head;
	while (node != NULL) {
		struct list_node *save = node;
		node = node->next;
		kfree(save);
	}
	kfree(list);
}

void listnode_destroy(struct list_node *node)
{
	kfree(node);
}

// Conversion

struct list *str_split(const char *str, const char *delim, uint32_t *numtokens)
{
	struct list *ret_list = list_create();
	char *s = strdup(str);
	char *token, *rest = s;
	while ((token = strsep(&rest, delim)) != NULL) {
		if (!strcmp(token, "."))
			continue;
		if (!strcmp(token, "..")) {
			if (list_size(ret_list) > 0)
				list_pop(ret_list);
			continue;
		}
		list_push(ret_list, strdup(token));
		if (numtokens)
			(*numtokens)++;
	}
	kfree(s);
	return ret_list;
}

char *list_to_str(struct list *list, const char *delim)
{
	char *ret = kmalloc(256);
	memset(ret, 0, 256);
	int len = 0, ret_len = 256;
	while (list_size(list) > 0) {
		char *temp = list_pop(list)->val;
		int len_temp = strlen(temp);
		if (len + len_temp + 1 + 1 > ret_len) {
			ret_len = ret_len * 2;
			/* ret = krealloc(ret, ret_len); */
			ret = kmalloc(ret_len);
			len = len + len_temp + 1;
		}
		strcat(ret, delim);
		strcat(ret, temp);
	}
	return ret;
}