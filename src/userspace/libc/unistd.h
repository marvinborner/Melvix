#ifndef MELVIX_UNISTD_H
#define MELVIX_UNISTD_H

#include <stdint.h>

u32 exec(char *path);

u32 spawn(char *path);

void exit(u32 code);

u32 get_pid();

u32 read(char *path, u32 offset, u32 count, u8 *buf);

u32 write(char *path, u32 offset, u32 count, u8 *buf);

// These should be somewhere else ig
u32 wait(u32 *status);
u32 wait_pid(u32 pid, u32 *status, u32 options);

#endif