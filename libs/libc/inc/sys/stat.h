// MIT License, Copyright(c) 2021 Marvin Borner

#ifndef SYS_STAT_H
#define SYS_STAT_H

#include <sys/types.h>

struct stat {
	ino_t st_ino;
	mode_t st_mode;
	nlink_t st_nlink;
	uid_t st_uid;
	gid_t st_gid;
	off_t st_size;
};

#endif
