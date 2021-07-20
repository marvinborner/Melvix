// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef TASKING_CALL_H
#define TASKING_CALL_H

#include <stdint.h>

typedef enum {
	CALL_INVALID,
	CALL_TEST,
} call_t;

uintptr_t syscall_handle(call_t call, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2,
			 uintptr_t arg3, uintptr_t arg4);

#endif
