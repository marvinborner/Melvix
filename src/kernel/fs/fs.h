#ifndef MELVIX_FS_H
#define MELVIX_FS_H

#include <stdint.h>

u32 get_file_size(char *path);
u32 read(char *path, u32 offset, u32 count, u8 *buf);
u32 write(char *path, u32 offset, u32 count, u8 *buf);
u8 *read_file(char *path); // Only for temp kernel reads

#endif