#include <kernel/system.h>
#include <kernel/fs/ext2/ext2.h>
#include <kernel/fs/vfs/vfs.h>
#include <kernel/lib/data/list.h>
#include <kernel/lib/data/generic_tree.h>
#include <kernel/memory/alloc.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/lib.h>

gtree_t *vfs_tree;
vfs_node_t *vfs_root;

uint32_t vfs_get_file_size(vfs_node_t *node)
{
	if (node && node->get_file_size) {
		return node->get_file_size(node);
	}
	return 0;
}

void vfs_db_listdir(char *name)
{
	vfs_node_t *n = file_open(name, 0);
	if (!n) {
		log("Could not list a directory that does not exist\n");
		return;
	}
	if (!n->listdir)
		return;
	char **files = n->listdir(n);
	char **save = files;
	while (*files) {
		log("%s ", *files);
		kfree(*files);
		files++;
	}
	kfree(save);
	log("\n");
}

void print_vfstree_recur(gtreenode_t *node, int parent_offset)
{
	if (!node)
		return;
	char *tmp = kmalloc(512);
	int len = 0;
	memset(tmp, 0, 512);
	for (int i = 0; i < parent_offset; ++i) {
		strcat(tmp, " ");
	}
	char *curr = tmp + strlen(tmp);
	struct vfs_entry *fnode = (struct vfs_entry *)node->value;
	if (fnode->file) {
		log(curr, "%s(0x%x, %s)", fnode->name, (uint32_t)fnode->file, fnode->file->name);
	} else {
		log(curr, "%s(empty)", fnode->name);
	}
	log("%s\n", tmp);
	len = strlen(fnode->name);
	kfree(tmp);
	foreach(child, node->children)
	{
		print_vfstree_recur(child->val, parent_offset + len + 1);
	}
}

void print_vfstree()
{
	print_vfstree_recur(vfs_tree->root, 0);
}

uint32_t vfs_read(vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer)
{
	if (node && node->read) {
		uint32_t ret = node->read(node, offset, size, buffer);
		return ret;
	}
	return -1;
}

uint32_t vfs_write(vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer)
{
	if (node && node->write) {
		uint32_t ret = node->write(node, offset, size, buffer);
		return ret;
	}
	return -1;
}

void vfs_open(struct vfs_node *node, uint32_t flags)
{
	if (!node)
		return;
	if (node->refcount >= 0)
		node->refcount++;
	node->open(node, flags);
}

void vfs_close(vfs_node_t *node)
{
	if (!node || node == vfs_root || node->refcount == -1)
		return;
	node->refcount--;
	if (node->refcount == 0)
		node->close(node);
}

void vfs_chmod(vfs_node_t *node, uint32_t mode)
{
	if (node->chmod)
		return node->chmod(node, mode);
}

struct dirent *vfs_readdir(vfs_node_t *node, uint32_t index)
{
	if (node && (node->flags & FS_DIRECTORY) && node->readdir)
		return node->readdir(node, index);
	return NULL;
}

vfs_node_t *vfs_finddir(vfs_node_t *node, char *name)
{
	if (node && (node->flags & FS_DIRECTORY) && node->finddir)
		return node->finddir(node, name);
	return NULL;
}

int vfs_ioctl(vfs_node_t *node, int request, void *argp)
{
	if (!node)
		return -1;
	if (node->ioctl)
		return node->ioctl(node, request, argp);
	return -1; // ENOTTY
}

void vfs_mkdir(char *name, unsigned short permission)
{
	// Find parent directory
	int i = strlen(name);
	char *dirname = strdup(name);
	char *save_dirname = dirname;
	char *parent_path = "/";
	while (i >= 0) {
		if (dirname[i] == '/') {
			if (i != 0) {
				dirname[i] = '\0';
				parent_path = dirname;
			}
			dirname = &dirname[i + 1];
			break;
		}
		i--;
	}

	// Open file
	vfs_node_t *parent_node = file_open(parent_path, 0);
	if (!parent_node) {
		kfree(save_dirname);
	}

	// mkdir
	if (parent_node->mkdir)
		parent_node->mkdir(parent_node, dirname, permission);
	kfree(save_dirname);

	vfs_close(parent_node);
}

int vfs_create_file(char *name, unsigned short permission)
{
	// Find parent directory
	int i = strlen(name);
	char *dirname = strdup(name);
	char *save_dirname = dirname;
	char *parent_path = "/";
	while (i >= 0) {
		if (dirname[i] == '/') {
			if (i != 0) {
				dirname[i] = '\0';
				parent_path = dirname;
			}
			dirname = &dirname[i + 1];
			break;
		}
		i--;
	}

	// Open file
	vfs_node_t *parent_node = file_open(parent_path, 0);
	if (!parent_node) {
		kfree(save_dirname);
		return -1;
	}
	if (parent_node->create)
		parent_node->create(parent_node, dirname, permission);
	kfree(save_dirname);
	vfs_close(parent_node);
	return 0;
}

void vfs_unlink(char *name)
{
	// Find parent directory
	int i = strlen(name);
	char *dirname = strdup(name);
	char *save_dirname = dirname;
	char *parent_path = "/";
	while (i >= 0) {
		if (dirname[i] == '/') {
			if (i != 0) {
				dirname[i] = '\0';
				parent_path = dirname;
			}
			dirname = &dirname[i + 1];
			break;
		}
		i--;
	}

	// Open file
	vfs_node_t *parent_node = file_open(parent_path, 0);
	if (!parent_node) {
		kfree(save_dirname);
	}
	if (parent_node->unlink)
		parent_node->unlink(parent_node, dirname);
	kfree(save_dirname);
	vfs_close(parent_node);
}

char *expand_path(char *input)
{
	list_t *input_list = str_split(input, "/", NULL);
	char *ret = list2str(input_list, "/");
	return ret;
}

vfs_node_t *get_mountpoint_recur(char **path, gtreenode_t *subroot)
{
	int found = 0;
	char *curr_token = strsep(path, "/");
	if (curr_token == NULL || !strcmp(curr_token, "")) {
		struct vfs_entry *ent = (struct vfs_entry *)subroot->value;
		return ent->file;
	}
	foreach(child, subroot->children)
	{
		gtreenode_t *tchild = (gtreenode_t *)child->val;
		struct vfs_entry *ent = (struct vfs_entry *)(tchild->value);
		if (strcmp(ent->name, curr_token) == 0) {
			found = 1;
			subroot = tchild;
			break;
		}
	}

	if (!found) {
		*path = curr_token;
		return ((struct vfs_entry *)(subroot->value))->file;
	}
	return get_mountpoint_recur(path, subroot);
}
vfs_node_t *get_mountpoint(char **path)
{
	if (strlen(*path) > 1 && (*path)[strlen(*path) - 1] == '/')
		*(path)[strlen(*path) - 1] = '\0';
	if (!*path || *(path)[0] != '/')
		return NULL;
	if (strlen(*path) == 1) {
		*path = '\0';
		struct vfs_entry *ent = (struct vfs_entry *)vfs_tree->root->value;
		return ent->file;
	}
	(*path)++;
	return get_mountpoint_recur(path, vfs_tree->root);
}

vfs_node_t *file_open(const char *file_name, uint32_t flags)
{
	char *curr_token = NULL;
	char *filename = strdup(file_name);
	char *free_filename = filename;
	char *save = strdup(filename);
	char *original_filename = filename;
	char *new_start = NULL;
	vfs_node_t *nextnode = NULL;
	vfs_node_t *startpoint = get_mountpoint(&filename);
	if (!startpoint)
		return NULL;
	if (filename)
		new_start = strstr(save + (filename - original_filename), filename);
	while (filename != NULL && ((curr_token = strsep(&new_start, "/")) != NULL)) {
		nextnode = vfs_finddir(startpoint, curr_token);
		if (!nextnode)
			return NULL;
		startpoint = nextnode;
	}
	if (!nextnode)
		nextnode = startpoint;
	vfs_open(nextnode, flags);
	kfree(save);
	kfree(free_filename);
	return nextnode;
}

void vfs_init()
{
	vfs_tree = tree_create();
	struct vfs_entry *root = kmalloc(sizeof(struct vfs_entry));
	root->name = strdup("root");
	root->file = NULL;
	tree_insert(vfs_tree, NULL, root);
}

void vfs_mount_dev(char *mountpoint, vfs_node_t *node)
{
	vfs_mount(mountpoint, node);
}

void vfs_mount_recur(char *path, gtreenode_t *subroot, vfs_node_t *fs_obj)
{
	int found = 0;
	char *curr_token = strsep(&path, "/");

	if (curr_token == NULL || !strcmp(curr_token, "")) {
		struct vfs_entry *ent = (struct vfs_entry *)subroot->value;
		if (ent->file) {
			log("The path is already mounted, plz unmount before mounting again\n");
			return;
		}
		if (!strcmp(ent->name, "/"))
			vfs_root = fs_obj;
		ent->file = fs_obj;
		return;
	}

	foreach(child, subroot->children)
	{
		gtreenode_t *tchild = (gtreenode_t *)child->val;
		struct vfs_entry *ent = (struct vfs_entry *)(tchild->value);
		if (strcmp(ent->name, curr_token) == 0) {
			found = 1;
			subroot = tchild;
		}
	}

	if (!found) {
		struct vfs_entry *ent = kcalloc(sizeof(struct vfs_entry), 1);
		ent->name = strdup(curr_token);
		subroot = tree_insert(vfs_tree, subroot, ent);
	}
	vfs_mount_recur(path, subroot, fs_obj);
}

void vfs_mount(char *path, vfs_node_t *fs_obj)
{
	fs_obj->refcount = -1;
	fs_obj->fs_type = 0;
	if (path[0] == '/' && strlen(path) == 1) {
		struct vfs_entry *ent = (struct vfs_entry *)vfs_tree->root->value;
		if (ent->file) {
			log("The path is already mounted, plz unmount before mounting again\n");
			return;
		}
		vfs_root = fs_obj;
		ent->file = fs_obj;
		return;
	}
	vfs_mount_recur(path + 1, vfs_tree->root, fs_obj);
}