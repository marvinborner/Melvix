// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef ASSERT_H
#define ASSERT_H

#include <print.h>

#ifdef KERNEL
#include <proc.h>
#define assert(exp)                                                                                \
	{                                                                                          \
		if (!(exp)) {                                                                      \
			printf("%s:%d: %s: Kernel assertion '%s' failed\n", __FILE__, __LINE__,    \
			       __func__, #exp);                                                    \
			while (1)                                                                  \
				__asm__ volatile("cli\nhlt");                                      \
		}                                                                                  \
	}
#elif defined(USER)
#define assert(exp)                                                                                \
	{                                                                                          \
		if (!(exp)) {                                                                      \
			err(1, "%s:%d: %s: Assertion '%s' failed\n", __FILE__, __LINE__, __func__, \
			    #exp);                                                                 \
		}                                                                                  \
	}
#endif

#define assert_not_reached()                                                                       \
	panic("%s:%d: %s: Reached code that should not be reached\n", __FILE__, __LINE__, __func__)

#endif
