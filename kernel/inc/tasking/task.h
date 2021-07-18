// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef TASKING_TASK_H
#define TASKING_TASK_H

#include <sys/types.h>

struct task {
	pid_t pid;
};

enum task_priv {
	PRIV_DEFAULT,
	PRIV_SUPER,
};

struct task *task_create(struct task *parent, const char *name, uintptr_t entry,
			 enum task_priv priv);

#endif
