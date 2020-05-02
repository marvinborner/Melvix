#include <stdint.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/ext2.h>
#include <kernel/lib/stdlib.h>
#include <kernel/memory/alloc.h>

uint32_t sys_read(char *path, uint32_t offset, uint32_t count, char *buf)
{
	struct fs_node *node = (struct fs_node *)umalloc(sizeof(struct fs_node));
	strcpy(node->name, path);
	ext2_root->open(node);
	if (node->inode != 0) {
		uint32_t size = ((struct ext2_file *)node->impl)->inode.size;
		ext2_root->read(node, 0, size, buf);
		buf[size - 1] = '\0';
		ext2_root->close(node);
		return size;
	} else {
		ext2_root->close(node);
		return -1;
	}
}