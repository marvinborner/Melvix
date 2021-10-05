/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#ifndef ASSERT_H
#define ASSERT_H

#define assert(exp) (void)(exp)

#if 0
#define assert(exp)                                                                                \
	{                                                                                          \
		if (!(exp)) {                                                                      \
			panic("%s:%d: %s: Assertion '%s' failed\n", __FILE__, __LINE__, __func__,  \
			      #exp);                                                               \
		}                                                                                  \
	}

#define assert_not_reached()                                                                       \
	panic("%s:%d: %s: Reached code that should not be reached\n", __FILE__, __LINE__, __func__)
#endif

#endif
