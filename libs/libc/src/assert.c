// MIT License, Copyright(c) 2021 Marvin Borner

#include <assert.h>

__attribute__((noreturn, weak)) void __assert_fail(const char *expression, const char *file,
						   unsigned int line, const char *function)
{
	(void)expression;
	(void)file;
	(void)line;
	(void)function;
	// TODO: Assert panic

	while (1)
		;
}
