#include <stdint.h>
#include <stddef.h>
#include <kernel/fs/vfs.h>

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
}

void fs_close(struct fs_node *node)
{
	if (node->close != NULL)
		node->close(node);
}

struct dirent *fs_read_directory(struct fs_node *node, uint32_t index)
{
	if ((node->type & DIR_NODE) != 0 && node->find_dir != NULL)
		return node->read_dir(node, index);
	else
		return (struct dirent *)NULL;
}

struct fs_node *fs_find_directory(struct fs_node *node, char *name)
{
	if ((node->type & DIR_NODE) != 0 && node->find_dir != NULL)
		return node->find_dir(node, name);
	else
		return (struct fs_node *)NULL;
}