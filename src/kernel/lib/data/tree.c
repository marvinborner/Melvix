#include <stdint.h>
#include <lib/data.h>
#include <memory/alloc.h>

struct tree *tree_create()
{
	return (struct tree *)kcalloc(sizeof(struct tree), 1);
}

struct tree_node *treenode_create(void *value)
{
	struct tree_node *n = kcalloc(sizeof(struct tree_node), 1);
	n->value = value;
	n->children = list_create();
	return n;
}

struct tree_node *tree_insert(struct tree *tree, struct tree_node *subroot, void *value)
{
	struct tree_node *treenode = kcalloc(sizeof(struct tree_node), 1);
	treenode->children = list_create();
	treenode->value = value;

	if (!tree->root) {
		tree->root = treenode;
		return treenode;
	}
	list_insert_front(subroot->children, treenode);
	return treenode;
}

struct tree_node *tree_find_parent(struct tree *tree, struct tree_node *remove_node,
				   int *child_index)
{
	if (remove_node == tree->root)
		return NULL;
	return tree_find_parent_recur(tree, remove_node, tree->root, child_index);
}

struct tree_node *tree_find_parent_recur(struct tree *tree, struct tree_node *remove_node,
					 struct tree_node *subroot, int *child_index)
{
	int idx;
	if ((idx = list_contain(subroot->children, remove_node)) != -1) {
		*child_index = idx;
		return subroot;
	}
	foreach(child, subroot->children)
	{
		struct tree_node *ret =
			tree_find_parent_recur(tree, remove_node, child->val, child_index);
		if (ret != NULL) {
			return ret;
		}
	}
	return NULL;
}

void tree_remove(struct tree *tree, struct tree_node *remove_node)
{
	int child_index = -1;
	struct tree_node *parent = tree_find_parent(tree, remove_node, &child_index);
	if (parent != NULL) {
		struct tree_node *freethis = list_remove_by_index(parent->children, child_index);
		kfree(freethis);
	}
}

void tree2list_recur(struct tree_node *subroot, struct list *list)
{
	if (subroot == NULL)
		return;
	foreach(child, subroot->children)
	{
		struct tree_node *curr_treenode = (struct tree_node *)child->val;
		void *curr_val = curr_treenode->value;
		list_insert_back(list, curr_val);
		tree2list_recur(child->val, list);
	}
}

void tree2list(struct tree *tree, struct list *list)
{
	tree2list_recur(tree->root, list);
}

void tree2array(struct tree *tree, void **array, int *size)
{
	tree2array_recur(tree->root, array, size);
}

void tree2array_recur(struct tree_node *subroot, void **array, int *size)
{
	if (subroot == NULL)
		return;
	void *curr_val = (void *)subroot->value;
	array[*size] = curr_val;
	*size = *size + 1;
	foreach(child, subroot->children)
	{
		tree2array_recur(child->val, array, size);
	}
}