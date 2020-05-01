#ifndef MELVIX_DEV_H
#define MELVIX_DEV_H

#include <stdint.h>
#include <kernel/fs/vfs.h>

struct dev {
	char name[128];
	uint32_t block_size;
	read read;
	write write;

	uint8_t bus;
	uint8_t drive;
};

void dev_make(char *name, read read, write write);
uint32_t dev_read(struct fs_node *dev, uint32_t offset, uint32_t size, char *buffer);
uint32_t dev_write(struct fs_node *dev, uint32_t offset, uint32_t size, char *buffer);

void dev_stdout();
void dev_stdin();
void dev_stderr();

#endif