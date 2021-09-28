// MIT License, Copyright (c) 2020 Marvin Borner

#include <list.h>
#include <mem.h>
#include <rand.h>
#include <str.h>

#include <core/protection.h>
#include <fs/virtual.h>

/**
 * VFS
 */

PROTECTED static struct list *mount_points = NULL;

char *vfs_normalize_path(const char *path)
{
	char *fixed = strdup(path); // TODO: Reduce allocations!
	int len = strlen(fixed);
	if (fixed[len - 1] == '/' && len != 1)
		fixed[len - 1] = '\0';
	return fixed;
}

// TODO: Reduce allocations in VFS find
static struct mount_info *vfs_recursive_find(char *path)
{
	struct node *iterator = mount_points->head;
	char *fixed = vfs_normalize_path(path);
	free(path); // Due to recursiveness

	while (iterator) {
		struct mount_info *m = iterator->data;
		if (!strcmp(m->path, fixed)) {
			free(fixed);
			return m;
		}
		iterator = iterator->next;
	}

	if (strlen(fixed) == 1) {
		free(fixed);
		return NULL;
	}

	*(strrchr(fixed, '/') + 1) = '\0';
	return vfs_recursive_find(fixed);
}

u8 vfs_mounted(struct vfs_dev *dev, const char *path)
{
	struct node *iterator = mount_points->head;
	while (iterator) {
		if (((struct mount_info *)iterator->data)->dev->id == dev->id ||
		    !strcmp(((struct mount_info *)iterator->data)->path, path))
			return 1;
		iterator = iterator->next;
	}
	return 0;
}

struct mount_info *vfs_find_mount_info(const char *path)
{
	stac();
	if (path[0] != '/') {
		clac();
		return NULL;
	}
	clac();
	struct mount_info *ret = vfs_recursive_find(EXPOSE(strdup, path));
	return ret;
}

u8 vfs_mount(struct vfs_dev *dev, const char *path)
{
	char *fixed = vfs_normalize_path(path);
	struct mount_info *m = malloc(sizeof(*m));
	m->path = fixed;
	m->dev = dev;
	list_add(mount_points, m);
	return 1;
}

TEMPORARY void vfs_add_dev(struct vfs_dev *dev)
{
	dev->id = rand() + 1;
}

struct vfs_dev *vfs_find_dev(const char *path)
{
	stac();
	if (path[0] != '/') {
		clac();
		return NULL;
	}
	clac();
	struct mount_info *m = vfs_find_mount_info(path);
	return m && m->dev ? m->dev : NULL;
}

TEMPORARY void vfs_load(struct vfs_dev *dev)
{
	/* if (!mbr_load(dev)) { */
	/* 	assert(ext2_load(dev)); */
	/* 	// TODO: Add GPT support */
	/* } */
}

TEMPORARY void vfs_install(void)
{
	mount_points = list_new();
}
