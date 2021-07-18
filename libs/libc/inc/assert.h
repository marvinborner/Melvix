// MIT License, Copyright(c) 2021 Marvin Borner

#ifndef ASSERT_H
#define ASSERT_H

__attribute__((noreturn, weak)) void __assert_fail(const char *expression, const char *file,
						   unsigned int line, const char *function);

#define assert(x) ((void)((x) || (__assert_fail(#x, __FILE__, __LINE__, __func__), 0)))

#endif
