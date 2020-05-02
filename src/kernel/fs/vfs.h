#ifndef MELVIX_VFS_H
#define MELVIX_VFS_H

#include <stdint.h>

#define MAX_NAME_LENGTH 128

enum node_type {
	FILE_NODE = 1,
	DIR_NODE,
	CHAR_DEV_NODE,
	BLOCK_DEV_NODE,
	PIPE_NODE,
	SYMLINK_NODE,
	MOUNTPOINT_NODE = 8
};

struct fs_node;
struct dirent;

typedef uint32_t (*read)(struct fs_node *, uint32_t, uint32_t, char *);
typedef uint32_t (*write)(struct fs_node *, uint32_t, uint32_t, char *);
typedef void (*open)(struct fs_node *);
typedef void (*close)(struct fs_node *);
typedef struct dirent *(*read_dir)(struct fs_node *, uint32_t);
typedef struct fs_node *(*find_dir)(struct fs_node *, char *);

struct fs_node {
	char name[MAX_NAME_LENGTH];
	uint32_t length;
	uint32_t inode;
	uint32_t permissions;
	uint32_t uid;
	uint32_t gid;
	enum node_type type;

	struct dev *dev;
	struct fs_node *node_ptr;
	struct fs_node *link;

	void *impl;

	read read;
	write write;
	open open;
	close close;
	read_dir read_dir;
	find_dir find_dir;
};

struct dirent {
	char name[MAX_NAME_LENGTH];
	uint32_t inode;
};

extern struct fs_node *fs_root;

uint32_t fs_read(struct fs_node *node, uint32_t offset, uint32_t size, char *buf);
uint32_t fs_write(struct fs_node *node, uint32_t offset, uint32_t size, char *buf);
void fs_open(struct fs_node *node);
void fs_close(struct fs_node *node);
struct dirent *fs_read_dir(struct fs_node *node, uint32_t index);
struct fs_node *fs_find_dir(struct fs_node *node, char *name);

char *basename(char *name);
void vfs_ls(char *path);
struct fs_node *vfs_get_dir(struct fs_node *node, char *name);
struct fs_node *vfs_touch(struct fs_node *node, char *name);
struct fs_node *vfs_mkdir(struct fs_node *node, char *name);

#endif