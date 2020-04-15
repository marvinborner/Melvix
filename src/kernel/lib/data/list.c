#include <kernel/lib/data/list.h>
#include <kernel/memory/alloc.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/lib.h>

list_t *list_create()
{
	list_t *list = kcalloc(sizeof(list_t), 1);
	return list;
}

uint32_t list_size(list_t *list)
{
	if (!list)
		return 0;
	return list->size;
}

void *list_remove_node(list_t *list, listnode_t *node)
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

listnode_t *list_insert_front(list_t *list, void *val)
{
	listnode_t *t = kcalloc(sizeof(listnode_t), 1);
	list->head->prev = t;
	t->next = list->head;
	t->val = val;

	if (!list->head)
		list->tail = t;

	list->head = t;
	list->size++;
	return t;
}

void list_insert_back(list_t *list, void *val)
{
	listnode_t *t = kcalloc(sizeof(listnode_t), 1);
	t->prev = list->tail;
	if (list->tail)
		list->tail->next = t;
	t->val = val;

	if (!list->head)
		list->head = t;

	list->tail = t;
	list->size++;
}

void *list_remove_front(list_t *list)
{
	if (!list->head)
		return 0;
	listnode_t *t = list->head;
	void *val = t->val;
	list->head = t->next;
	if (list->head)
		list->head->prev = NULL;
	kfree(t);
	list->size--;
	return val;
}

void *list_remove_back(list_t *list)
{
	if (!list->head)
		return NULL;
	listnode_t *t = list->tail;
	void *val = t->val;
	list->tail = t->prev;
	if (list->tail)
		list->tail->next = NULL;
	kfree(t);
	list->size--;
	return val;
}

void list_push(list_t *list, void *val)
{
	list_insert_back(list, val);
}

listnode_t *list_pop(list_t *list)
{
	if (!list->head)
		return NULL;
	listnode_t *t = list->tail;
	list->tail = t->prev;
	if (list->tail)
		list->tail->next = NULL;
	list->size--;
	return t;
}

void list_enqueue(list_t *list, void *val)
{
	list_insert_front(list, val);
}

listnode_t *list_dequeue(list_t *list)
{
	return list_pop(list);
}

void *list_peek_front(list_t *list)
{
	if (!list->head)
		return NULL;
	return list->head->val;
}

void *list_peek_back(list_t *list)
{
	if (!list->tail)
		return NULL;
	return list->tail->val;
}

int list_contain(list_t *list, void *val)
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

listnode_t *list_get_node_by_index(list_t *list, int index)
{
	if (index < 0 || (uint32_t)index >= list_size(list))
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

void *list_remove_by_index(list_t *list, int index)
{
	listnode_t *node = list_get_node_by_index(list, index);
	return list_remove_node(list, node);
}

void list_destroy(list_t *list)
{
	listnode_t *node = list->head;
	while (node != NULL) {
		listnode_t *save = node;
		node = node->next;
		kfree(save);
	}
	kfree(list);
}

void listnode_destroy(listnode_t *node)
{
	kfree(node);
}

char *list2str(list_t *list, const char *delim)
{
	char *ret = kmalloc(256);
	memset(ret, 0, 256);
	int len = 0, ret_len = 256;
	while (list_size(list) > 0) {
		char *temp = list_pop(list)->val;
		int len_temp = strlen(temp);
		if (len + len_temp + 1 + 1 > ret_len) {
			ret_len = ret_len * 2;
			ret = krealloc(ret, ret_len);
			len = len + len_temp + 1;
		}
		strcat(ret, delim);
		strcat(ret, temp);
	}
	return ret;
}

list_t *str_split(const char *str, const char *delim, uint32_t *numtokens)
{
	list_t *ret_list = list_create();
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