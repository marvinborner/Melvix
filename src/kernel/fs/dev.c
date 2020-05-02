#include <stdint.h>
#include <stddef.h>
#include <kernel/system.h>
#include <kernel/fs/dev.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/ext2.h>
#include <kernel/memory/alloc.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>

void dev_make(char *name, read read, write write)
{
	struct fs_node *dev_node = (struct fs_node *)kmalloc(sizeof(struct fs_node));
	strcpy(dev_node->name, "/dev");
	fs_open(dev_node);

	if (dev_node->inode == 0) {
		warn("Can't make device, no path for /dev");
		return;
	}

	fs_close(dev_node);

	char *path = (char *)kmalloc(strlen(name) + strlen("/dev/") + 2);
	strcpy(path, "/dev/");
	strcpy(&path[strlen("/dev/")], name);

	// TODO: Touch dev files in the vfs instead of opening it via ext2
	struct fs_node *node = (struct fs_node *)kmalloc(sizeof(struct fs_node));
	strcpy(node->name, path);
	fs_open(node);
	kfree(path);

	if (node->inode == 0) {
		warn("Couldn't resolve path");
		return;
	}

	node->read = read;
	node->write = write;

	node->dev = (struct dev *)kmalloc(sizeof(struct dev));
	node->dev->read = read;
	node->dev->write = write;
	node->dev->block_size = 512;
}

uint32_t dev_read(struct fs_node *dev, uint32_t offset, uint32_t size, char *buffer)
{
	if (dev->read == NULL) {
		warn("Can't read from device");
		return (uint32_t)-1;
	}

	return dev->read(dev, offset, size, buffer);
}

uint32_t dev_write(struct fs_node *dev, uint32_t offset, uint32_t size, char *buffer)
{
	if (dev->write == NULL) {
		warn("Can't write to device");
		return (uint32_t)-1;
	}

	return dev->write(dev, offset, size, buffer);
}

// Standard devices

uint32_t stdin_read(struct fs_node *node, uint32_t offset, uint32_t size, uint8_t *buffer)
{
	// Do something
	return 0;
}

void dev_stdin()
{
	dev_make("stdout", (read)stdin_read, NULL);
	struct fs_node *node = (struct fs_node *)kmalloc(sizeof(struct fs_node));
	strcpy(node->name, "/dev/stdin");
	fs_open(node);
	node->dev->block_size = 0;
	node->length = 0xFFFFFFFF;
}

uint32_t stdout_write(struct fs_node *node, uint32_t offset, uint32_t size, uint8_t *buffer)
{
	//putch(*buffer);
	return 0;
}

void dev_stdout()
{
	dev_make("stdout", NULL, (write)stdout_write);
	struct fs_node *node = (struct fs_node *)kmalloc(sizeof(struct fs_node));
	strcpy(node->name, "/dev/stdout");
	fs_open(node);
	node->dev->block_size = 0;
}

uint32_t stderr_write(struct fs_node *node, uint32_t offset, uint32_t size, uint8_t *buffer)
{
	//putch(*buffer);
	return 0;
}

void dev_stderr()
{
	dev_make("stderr", NULL, (write)stderr_write);
	struct fs_node *node = (struct fs_node *)kmalloc(sizeof(struct fs_node));
	strcpy(node->name, "/dev/stderr");
	fs_open(node);
	node->dev->block_size = 0;
}