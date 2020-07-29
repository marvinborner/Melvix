// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef ASSERT_H
#define ASSERT_H

#include <print.h>

#define assert(exp)                                                                                \
	(exp) ? 0 : printf("%s:%d: %s: Assertion '%s' failed\n", __FILE__, __LINE__, __func__, #exp)

#endif
