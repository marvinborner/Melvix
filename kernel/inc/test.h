// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef TEST_H
#define TEST_H

#include <boot.h>
#include <print.h>
#include <str.h>

#define a_mag 0x55
#define b_mag 0x42

void pass_or_fail(const char *file_name, int line_num, const char *func, const char *first,
		  const char *second, int success);

#define check(exp) pass_or_fail(__FILE__, __LINE__, __func__, #exp, "1", exp);
#define equals(first, second)                                                                      \
	pass_or_fail(__FILE__, __LINE__, __func__, #first, #second, (first) == (second));
#define equals_str(first, second)                                                                  \
	pass_or_fail(__FILE__, __LINE__, __func__, #first, #second, strcmp((first), (second)) == 0);

void test_all(struct vid_info *vid_info);

#endif
