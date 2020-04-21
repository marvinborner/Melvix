#ifndef MELVIX_VFS_H
#define MELVIX_VFS_H

#include <stdbool.h>
#include <stddef.h>
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
struct dir_entry;

typedef uint32_t (*read)(struct fs_node *, size_t, size_t, char *);
typedef uint32_t (*write)(struct fs_node *, size_t, size_t, char *);
typedef void (*open)(struct fs_node *);
typedef void (*close)(struct fs_node *);

typedef struct dir_entry *(*read_dir)(struct fs_node *, size_t);
typedef struct fs_node *(*find_dir)(struct fs_node *, char *);

struct fs_node {
	char name[MAX_NAME_LENGTH];
	uint32_t length;
	uint32_t inode;
	uint32_t permissions;
	uint32_t uid;
	uint32_t gid;
	enum node_type type;

	struct fs_node *node_ptr;

	void *impl;

	read read;
	write write;
	open open;
	close close;
	read_dir read_dir;
	find_dir find_dir;
};

struct dir_entry {
	char name[MAX_NAME_LENGTH];
	uint32_t inode;
};

struct fs_node *fs_root;

uint32_t read_fs_node(struct fs_node *node, size_t offset, size_t size, char *buf);
uint32_t write_fs_node(struct fs_node *node, size_t offset, size_t size, char *buf);
void open_fs_node(struct fs_node *node, bool read, bool write);
void close_fs_node(struct fs_node *node);
struct dir_entry *read_dir_node(struct fs_node *node, size_t index);
struct fs_node *find_dir_node(struct fs_node *node, char *name);

#endif