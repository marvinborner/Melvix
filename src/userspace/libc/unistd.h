#ifndef MELVIX_UNISTD_H
#define MELVIX_UNISTD_H

#include <stdint.h>

u32 exec(char *path);

void exit(u32 code);

u32 fork();

u32 get_pid();

u32 sys_read(char *path, u32 offset, u32 count, char *buf);

u32 sys_write(char *path, u32 offset, u32 count, char *buf);

#endif