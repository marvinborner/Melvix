#include <fs/ext2.h>
#include <memory/alloc.h>
#include <stdint.h>
#include <system.h>

u32 get_file_size(char *path)
{
	u32 inode = ext2_look_up_path(path);
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
u32 read(char *path, u32 offset, u32 count, u8 *buf)
{
	u32 inode = ext2_look_up_path(path);
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
u32 write(char *path, u32 offset, u32 count, u8 *buf)
{
	warn("Writing is not supported!");
	return -1;
}

u8 *read_file(char *path)
{
	u32 inode = ext2_look_up_path(path);
	struct ext2_file file;
	ext2_open_inode(inode, &file);
	if (inode != 0) {
		u32 size = file.inode.size;
		debug("Reading %s: %dKiB", path, size >> 10);
		u8 *buf = kmalloc(size);
		ext2_read(&file, buf, size);
		kfree(file.buf);
		buf[size - 1] = '\0';
		return buf;
	} else {
		warn("File not found");
		return NULL;
	}
}