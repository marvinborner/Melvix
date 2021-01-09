// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef ASSERT_H
#define ASSERT_H

#include <print.h>

#ifdef kernel
#include <proc.h>
#define assert(exp)                                                                                \
	if (!(exp)) {                                                                              \
		printf("%s:%d: %s: Kernel assertion '%s' failed\n", __FILE__, __LINE__, __func__,  \
		       #exp);                                                                      \
		struct proc *assert_proc = proc_current();                                         \
		if (assert_proc)                                                                   \
			proc_exit(assert_proc, 1);                                                 \
		else                                                                               \
			__asm__ volatile("cli\nhlt");                                              \
	}
#elif defined(userspace)
#include <sys.h>
#define assert(exp)                                                                                \
	if (!(exp)) {                                                                              \
		printf("%s:%d: %s: Assertion '%s' failed\n", __FILE__, __LINE__, __func__, #exp);  \
		exit(1);                                                                           \
	}
#else
#error "No lib target specified. Please use -Dkernel or -Duserspace"
#endif

#endif
