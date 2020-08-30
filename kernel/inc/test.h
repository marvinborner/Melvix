// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef TEST_H
#define TEST_H

#include <boot.h>
#include <print.h>
#include <str.h>

#define a_mag 0x55
#define b_mag 0x42

#define check(exp)                                                                                 \
	if ((exp)) {                                                                               \
		printf("\x1B[32m[PASS]\x1B[0m %s:%d: %s: Test '%s'\n", __FILE__, __LINE__,         \
		       __func__, #exp);                                                            \
	} else {                                                                                   \
		printf("\x1B[31m[FAIL]\x1B[0m %s:%d: %s: Test '%s'\n", __FILE__, __LINE__,         \
		       __func__, #exp);                                                            \
	}

#define equals(first, second)                                                                      \
	if ((first) == (second)) {                                                                 \
		printf("\x1B[32m[PASS]\x1B[0m %s:%d: %s: Test equality '%s'(%d) == '%s'(%d)\n",    \
		       __FILE__, __LINE__, __func__, #first, (first), #second, (second));          \
	} else {                                                                                   \
		printf("\x1B[31m[FAIL]\x1B[0m %s:%d: %s: Test equality '%s'(%d) == '%s'(%d)\n",    \
		       __FILE__, __LINE__, __func__, #first, (first), #second, (second));          \
	}

#define equals_str(first, second)                                                                  \
	if (strcmp((first), (second)) == 0) {                                                      \
		printf("\x1B[32m[PASS]\x1B[0m %s:%d: %s: Test equality %s(%s) '%s'(%s)\n",         \
		       __FILE__, __LINE__, __func__, #first, (first), #second, (second));          \
	} else {                                                                                   \
		printf("\x1B[31m[FAIL]\x1B[0m %s:%d: %s: Test equality %s(%s) '%s'(%s)\n",         \
		       __FILE__, __LINE__, __func__, #first, (first), #second, (second));          \
	}

void test_all(struct vid_info *vid_info);

#endif
