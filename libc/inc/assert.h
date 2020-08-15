// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef ASSERT_H
#define ASSERT_H

#include <print.h>

#ifdef kernel
#define assert(exp)                                                                                \
	if (!(exp)) {                                                                              \
		printf("%s:%d: %s: Assertion '%s' failed\n", __FILE__, __LINE__, __func__, #exp);  \
		__asm__ volatile("cli\nhlt");                                                      \
	}
#elif defined(userspace)
#include <sys.h>
#define assert(exp)                                                                                \
	if (!(exp)) {                                                                              \
		printf("%s:%d: %s: Assertion '%s' failed\n", __FILE__, __LINE__, __func__, #exp);  \
		sys0(SYS_LOOP);                                                                    \
	}
#else
#error "No lib target specified. Please use -Dkernel or -Duserspace"
#endif

#endif
