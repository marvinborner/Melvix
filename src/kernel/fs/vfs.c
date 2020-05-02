#include <stdint.h>
#include <stddef.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/ext2.h>
#include <kernel/lib/stdlib.h>
#include <kernel/memory/alloc.h>
#include <kernel/system.h>

struct fs_node *fs_root = NULL;

uint32_t fs_read(struct fs_node *node, uint32_t offset, uint32_t size, char *buf)
{
	if (node->read != NULL)
		return node->read(node, offset, size, buf);
	else
		return 0;
}

uint32_t fs_write(struct fs_node *node, uint32_t offset, uint32_t size, char *buf)
{
	if (node->write != NULL)
		return node->write(node, offset, size, buf);
	else
		return 0;
}

void fs_open(struct fs_node *node)
{
	if (node->open != NULL)
		node->open(node);
	else // TODO: Better ext2 default open workaround
		fs_root->open(node);
}

void fs_close(struct fs_node *node)
{
	if (node->close != NULL)
		node->close(node);
	else
		fs_root->open(node);
}

struct dirent *fs_read_dir(struct fs_node *node, uint32_t index)
{
	if ((node->type & DIR_NODE) != 0 && node->find_dir != NULL)
		return node->read_dir(node, index);
	else
		return (struct dirent *)NULL;
}

struct fs_node *fs_find_dir(struct fs_node *node, char *name)
{
	if ((node->type & DIR_NODE) != 0 && node->find_dir != NULL)
		return node->find_dir(node, name);
	else
		return (struct fs_node *)NULL;
}

// TODO: This should be somewhere else ig
char *next_str(char *str)
{
	str = &str[strlen(str)];

	int i = 0;
	while (str[i] == 0)
		i++;

	return &str[i];
}

// By far not the fastest algorithm
struct fs_node *fs_path(struct fs_node *node, char *name)
{
	if (name[0] == '/')
		return node;

	char *cpy = (char *)kmalloc(strlen(name) + 2);
	strcpy(cpy, name);
	cpy[strlen(name) + 1] = 1;

	name = cpy;
	char *end = &name[strlen(name)];

	int len = strlen(name);
	for (int i = 0; i < len; i++)
		if (name[i] == '/')
			name[i] = 0;

	if (strlen(name) == 0)
		name = next_str(name);

	do {
		node = node->node_ptr;

		while (node != NULL) {
			if (strcmp(node->name, name) == 0)
				break;

			node = node->link;
		}

		if (node == NULL) {
			kfree(cpy);
			return NULL;
		}

		name = next_str(name);
	} while ((uint32_t)name < (uint32_t)end);

	kfree(cpy);
	return node;
}

void vfs_ls(char *path)
{
	struct fs_node *dir = fs_path(fs_root, path);
	if (dir == NULL) {
		warn("Invalid path");
		return;
	}

	struct fs_node *link = dir->node_ptr;

	log("Listing for %s", path);
	while (link != NULL) {
		log(link->name);
		link = link->link;
	}
}

struct fs_node *vfs_get_dir(struct fs_node *node, char *name)
{
	char *cpy = (char *)kmalloc(strlen(name) + 2);
	char *ref = cpy;
	strcpy(cpy, name);
	cpy[strlen(name) + 1] = 1;
	name = cpy;

	for (int i = strlen(name); i >= 0; i--)
		if (name[i] == '/') {
			name[i] = 0;
			break;
		}

	struct fs_node *output;
	if (strlen(name) == 0)
		output = fs_root;
	else
		output = fs_path(node, name);

	kfree(ref);
	return output;
}

char *basename(char *name)
{
	int i;
	for (i = strlen(name); i >= 0; i--)
		if (name[i] == '/')
			return &name[i + 1];

	return name;
}

// TODO: Implement file touching in ext2
struct fs_node *vfs_touch(struct fs_node *node, char *name)
{
	struct fs_node *dir = vfs_get_dir(node, name);

	if (dir == NULL)
		return NULL;

	struct fs_node *file = (struct fs_node *)kmalloc(sizeof(struct fs_node));
	ext2_node_init(file);
	file->type = FILE_NODE;
	strcpy(file->name, basename(name));

	//ext2_file *entry = ext2_new_file();
	//strcpy(file->name, entry->name);

	file->link = dir->node_ptr;
	dir->node_ptr = file;

	return file;
}

struct fs_node *vfs_mkdir(struct fs_node *node, char *name)
{
	struct fs_node *dir = vfs_get_dir(node, name);
	if (dir == NULL) {
		warn("Invalid path");
		return NULL;
	}

	struct fs_node *file = (struct fs_node *)kmalloc(sizeof(struct fs_node));
	ext2_node_init(file);
	file->type = DIR_NODE;
	strcpy(file->name, basename(name));

	file->link = dir->node_ptr;
	dir->node_ptr = file;

	return file;
}