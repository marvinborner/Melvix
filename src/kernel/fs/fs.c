#include <stdint.h>
#include <kernel/fs/ext2.h>
#include <kernel/system.h>
#include <kernel/memory/alloc.h>

uint32_t get_file_size(char *path)
{
	uint32_t inode = ext2_look_up_path(path);
	struct ext2_file file;
	ext2_open_inode(inode, &file);
	if (inode != 0) {
		return file.inode.size;
	} else {
		warn("File not found");
		return -1;
	}
}

// TODO: Implement offset
uint32_t read(char *path, uint32_t offset, uint32_t count, uint8_t *buf)
{
	uint32_t inode = ext2_look_up_path(path);
	struct ext2_file file;
	ext2_open_inode(inode, &file);
	if (inode != 0) {
		debug("Reading %s: %dKiB", path, count >> 10);
		ext2_read(&file, buf, count);
		kfree(file.buf);
		buf[count - 1] = '\0';
		return buf;
	} else {
		warn("File not found");
		return -1;
	}
}

// TODO: Implement writing
uint32_t write(char *path, uint32_t offset, uint32_t count, uint8_t *buf)
{
	warn("Writing is not supported!");
	return -1;
}

uint8_t *read_file(char *path)
{
	uint32_t inode = ext2_look_up_path(path);
	struct ext2_file file;
	ext2_open_inode(inode, &file);
	if (inode != 0) {
		size_t size = file.inode.size;
		debug("Reading %s: %dKiB", path, size >> 10);
		uint8_t *buf = kmalloc(size);
		ext2_read(&file, buf, size);
		kfree(file.buf);
		buf[size - 1] = '\0';
		return buf;
	} else {
		warn("File not found");
		return NULL;
	}
}