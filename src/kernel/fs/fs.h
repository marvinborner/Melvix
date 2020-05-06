#ifndef MELVIX_FS_H
#define MELVIX_FS_H

#include <stdint.h>

uint32_t get_file_size(char *path);
uint32_t read(char *path, uint32_t offset, uint32_t count, uint8_t *buf);
uint32_t write(char *path, uint32_t offset, uint32_t count, uint8_t *buf);
uint8_t *read_file(char *path); // Only for temp kernel reads

#endif