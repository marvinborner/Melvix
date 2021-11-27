/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#ifndef ERR_H
#define ERR_H

#include <def.h>

typedef enum {
	ERR_OK,
	ERR_NOT_FOUND,
	ERR_NOT_SUPPORTED,
	ERR_INVALID_ARGUMENTS,
} err;

const char *format_error(err code);

#define errno (*__errno())
extern u32 *__errno(void);

#define return_errno(__num)                                                                        \
	{                                                                                          \
		errno = __num;                                                                     \
		return -errno;                                                                     \
	}

#endif
