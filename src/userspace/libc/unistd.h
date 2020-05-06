#ifndef MELVIX_UNISTD_H
#define MELVIX_UNISTD_H

#include <stdint.h>

u32 exec(char *path);

void exit(u32 code);

u32 fork();

u32 get_pid();

u32 read(char *path, u32 offset, u32 count, u8 *buf);

u32 write(char *path, u32 offset, u32 count, u8 *buf);

#endif