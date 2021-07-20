// MIT License, Copyright(c) 2021 Marvin Borner
// Only for syscall_ret_ternal use

#ifndef SYSCALL_H
#define SYSCALL_H

#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/call.h>

typedef const long syscall_arg_t;
typedef long syscall_ret_t;

static inline syscall_ret_t syscall0(syscall_t num)
{
	syscall_ret_t a;
	__asm__ volatile("int $0x80" : "=a"(a) : "0"(num));
	return a;
}

static inline syscall_ret_t syscall1(syscall_t num, syscall_arg_t arg0)
{
	syscall_ret_t a;
	__asm__ volatile("int $0x80" : "=a"(a) : "0"(num), "b"(arg0));
	return a;
}

static inline syscall_ret_t syscall2(syscall_t num, syscall_arg_t arg0, syscall_arg_t arg1)
{
	syscall_ret_t a;
	__asm__ volatile("int $0x80" : "=a"(a) : "0"(num), "b"(arg0), "c"(arg1));
	return a;
}

static inline syscall_ret_t syscall3(syscall_t num, syscall_arg_t arg0, syscall_arg_t arg1,
				     syscall_arg_t arg2)
{
	syscall_ret_t a;
	__asm__ volatile("int $0x80" : "=a"(a) : "0"(num), "b"(arg0), "c"(arg1), "d"(arg2));
	return a;
}

static inline syscall_ret_t syscall4(syscall_t num, syscall_arg_t arg0, syscall_arg_t arg1,
				     syscall_arg_t arg2, syscall_arg_t arg3)
{
	syscall_ret_t a;
	__asm__ volatile("int $0x80"
			 : "=a"(a)
			 : "0"(num), "b"(arg0), "c"(arg1), "d"(arg2), "S"(arg3));
	return a;
}

static inline syscall_ret_t syscall5(syscall_t num, syscall_arg_t arg0, syscall_arg_t arg1,
				     syscall_arg_t arg2, syscall_arg_t arg3, syscall_arg_t arg4)
{
	syscall_ret_t a;
	__asm__ volatile("int $0x80"
			 : "=a"(a)
			 : "0"(num), "b"(arg0), "c"(arg1), "d"(arg2), "S"(arg3), "D"(arg4));
	return a;
}

static inline syscall_ret_t syscall_ret(syscall_ret_t ret)
{
	if (ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}

#define SYSCALL_CAST(val) ((syscall_arg_t)val)

#define syscall1(n, a) syscall1(n, SYSCALL_CAST(a))
#define syscall2(n, a, b) syscall2(n, SYSCALL_CAST(a), SYSCALL_CAST(b))
#define syscall3(n, a, b, c) syscall3(n, SYSCALL_CAST(a), SYSCALL_CAST(b), SYSCALL_CAST(c))
#define syscall4(n, a, b, c, d)                                                                    \
	syscall4(n, SYSCALL_CAST(a), SYSCALL_CAST(b), SYSCALL_CAST(c), SYSCALL_CAST(d))
#define syscall5(n, a, b, c, d, e)                                                                 \
	syscall5(n, SYSCALL_CAST(a), SYSCALL_CAST(b), SYSCALL_CAST(c), SYSCALL_CAST(d),            \
		 SYSCALL_CAST(e))

#define SYSCALL_COUNT_X(a, b, c, d, e, f, g, ...) g
#define SYSCALL_COUNT(...) SYSCALL_COUNT_X(__VA_ARGS__, 5, 4, 3, 2, 1, 0, )
#define SYSCALL_CONCAT_X(a, b) a##b
#define SYSCALL_CONCAT(a, b) SYSCALL_CONCAT_X(a, b)
#define SYSCALL_GET(b, ...) SYSCALL_CONCAT(b, SYSCALL_COUNT(__VA_ARGS__))(__VA_ARGS__)

#define __syscall(...) SYSCALL_GET(syscall, __VA_ARGS__)
#define syscall(...) syscall_ret(__syscall(__VA_ARGS__))

#endif
