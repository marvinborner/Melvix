// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef TASKING_CALL_H
#define TASKING_CALL_H

#include <stdint.h>
#include <sys/call.h>

uintptr_t syscall_handle(syscall_t call, syscall_arg_t arg0, syscall_arg_t arg1, syscall_arg_t arg2,
			 syscall_arg_t arg3, syscall_arg_t arg4);

#endif
