// MIT License, Copyright (c) 2020 Marvin Borner
// TODO: Add GUI and sys tests

#include "test.h"

#include <conv.h>
#include <crypto.h>
#include <list.h>
#include <math.h>
#include <mem.h>
#include <print.h>
#include <stack.h>
#include <str.h>
#include <sys.h>

#define TEST(name) static void test_##name(void)
#define EXEC(name) test_##name()
#define CHECK(exp) pass_or_fail(__FILE__, __LINE__, __func__, #exp, "1", (u8)(exp));
#define EQUALS(first, second)                                                                      \
	pass_or_fail(__FILE__, __LINE__, __func__, #first, #second, (u32)(first) == (u32)(second));
#define EQUALS_APPROX(first, second)                                                               \
	pass_or_fail(__FILE__, __LINE__, __func__, #first, #second,                                \
		     FABS((f64)(first) - (f64)(second)) < 0.0000005);
#define EQUALS_STR(first, second)                                                                  \
	pass_or_fail(__FILE__, __LINE__, __func__, #first, #second,                                \
		     strcmp((const char *)(first), (const char *)(second)) == 0);

static u32 failed;

static void pass_or_fail(const char *file_name, int line_num, const char *func, const char *first,
			 const char *second, int success)
{
	failed += success ? 0 : 1;
	log("\x1B[%s\x1B[0m %s:%d: %s: %s == %s\n", success ? "32m[PASS]" : "31m[FAIL]", file_name,
	    line_num, func, first, second);
}

TEST(conv)
{
	char buf[10];
	memset(buf, 0xff, sizeof(buf));

	itoa(42, buf, 2);
	EQUALS_STR(buf, "101010");

	itoa(42, buf, 16);
	EQUALS_STR(buf, "2a");

	itoa(-42, buf, 16);
	EQUALS_STR(buf, "ffffffd6");

	itoa(-42, buf, 10);
	EQUALS_STR(buf, "-42");

	memset(buf, 0xff, sizeof(buf));

	ftoa(0.123456789, buf, 5);
	EQUALS_STR(buf, "0.12345");

	ftoa(-0.123456789, buf, 5);
	EQUALS_STR(buf, "-0.12345");
}

TEST(crypto)
{
	const char *text = "Melvix";
	u32 length = 6;

	EQUALS(crc32(0, text, length), 0x98bb3595);

	const u8 md5_text[16] = {
		0x01, 0xdc, 0xaf, 0x55, 0x2a, 0xe5, 0x7a, 0xf2,
		0xe5, 0xb4, 0x75, 0xac, 0x0f, 0x38, 0x97, 0x9c,
	};
	u8 md5_res[16] = { 0 };
	md5(text, length, md5_res);
	EQUALS(memcmp(md5_res, md5_text, 16), 0);
}

TEST(def)
{
	EQUALS(MIN(2, 4), 2);
	EQUALS(MIN(4, -2), -2);
	EQUALS(MIN(0, -1), -1);
	EQUALS(MAX(2, 4), 4);
	EQUALS(MAX(4, -2), 4);
	EQUALS(MAX(0, -1), 0);

	EQUALS(ABS(42), 42);
	EQUALS(ABS(-42), 42);
	EQUALS(ABS(-0), 0);
	EQUALS(FABS(42.0), 42.0);
	EQUALS(FABS(-42.0), 42.0);
	EQUALS(FABS(-0.0), 0.0);

	const u8 arr0[] = { 1, 2, 3, 4 };
	EQUALS(COUNT(arr0), 4);
	UNUSED(arr0);

	const u8 arr1[] = { 0 };
	EQUALS(COUNT(arr1), 1);
	UNUSED(arr1);

	EQUALS(UPPER('0'), 0);
	EQUALS(UPPER('a'), 0);
	EQUALS(UPPER('A'), 1);
	EQUALS(UPPER('Z'), 1);

	EQUALS(LOWER('0'), 0);
	EQUALS(LOWER('A'), 0);
	EQUALS(LOWER('a'), 1);
	EQUALS(LOWER('z'), 1);

	EQUALS(ALPHA('0'), 0);
	EQUALS(ALPHA('a'), 1);
	EQUALS(ALPHA('z'), 1);
	EQUALS(ALPHA('A'), 1);
	EQUALS(ALPHA('Z'), 1);

	EQUALS(NUMERIC(')'), 0);
	EQUALS(NUMERIC('a'), 0);
	EQUALS(NUMERIC('Z'), 0);
	EQUALS(NUMERIC('0'), 1);
	EQUALS(NUMERIC('9'), 1);

	EQUALS(ALPHANUMERIC(')'), 0);
	EQUALS(ALPHANUMERIC('0'), 1);
	EQUALS(ALPHANUMERIC('9'), 1);
	EQUALS(ALPHANUMERIC('a'), 1);
	EQUALS(ALPHANUMERIC('z'), 1);
	EQUALS(ALPHANUMERIC('A'), 1);
	EQUALS(ALPHANUMERIC('Z'), 1);

#define TEST_STRING abc
	EQUALS_STR(STRINGIFY_PARAM(TEST_STRING), "TEST_STRING");
	EQUALS_STR(STRINGIFY(TEST_STRING), "abc");

	EQUALS(ALIGN_UP(1, 2), 2);
	EQUALS(ALIGN_UP(1, 16), 16);
	EQUALS(ALIGN_UP(32, 16), 32);

	EQUALS(ALIGN_DOWN(1, 2), 0);
	EQUALS(ALIGN_DOWN(32, 16), 32);
	EQUALS(ALIGN_DOWN(31, 16), 16);
}

TEST(list)
{
	int data0, data1, data2;

	struct list *list = list_new();
	CHECK(!!list);
	EQUALS(list->head, NULL);
	list_add(list, &data0);
	list_add(list, &data1);
	list_add(list, &data2);
	list_add(list, &data1);
	list_add(list, &data0);
	EQUALS(list->head->data, &data0);
	list_remove(list, list->head);
	EQUALS(list->head->data, &data1);
	EQUALS(list->head->next->data, &data2);
	list_swap(list, list->head, list->head->next);
	EQUALS(list->head->data, &data2);
	EQUALS(list->head->next->data, &data1);
	EQUALS(list_first_data(list, &data0), list->head->next->next->next);
	EQUALS(list->head->next->next->next->next, NULL);

	struct node *last = list_last(list);
	CHECK(!!last);
	if (last)
		EQUALS(list_last(list)->data, &data0);

	list_destroy(list);
}

TEST(math)
{
	EQUALS(mpow(2, 3), 8.0);

	EQUALS_APPROX(msqrt(2), 1.4142135);
	EQUALS(msqrt(-2), 0.0); // Well...

	EQUALS_APPROX(msin(M_PI), 0.0);
	EQUALS_APPROX(msin(1234), 0.601928);
	EQUALS_APPROX(mcos(1234), -0.798551);
	EQUALS_APPROX(mtan(1234), -0.753775);
	EQUALS_APPROX(msqrt(1234), 35.128336);
	EQUALS_APPROX(msin(-1), -0.8414709848078965);
	EQUALS_APPROX(mcos(-1), 0.5403023058681398);
	EQUALS_APPROX(mtan(-1), -1.5574077246549023);
}

TEST(mem)
{
	u8 *zero = malloc(1);
	CHECK(!!zero);
	zero[0] = 42;
	EQUALS(zero[0], 42);
	free(zero);

	u8 *test = zalloc(0x1000);
	CHECK(mememp(test, 0x1000));
	free(test);

	u8 nonemp[] = { 0, 0, 0, 0, 1 };
	CHECK(!mememp(nonemp, sizeof(nonemp)));
	nonemp[sizeof(nonemp) - 1] = 0;
	CHECK(mememp(nonemp, sizeof(nonemp)));

	EQUALS(memcmp("abc", "cba", 3), -1);
	EQUALS(memcmp("cba", "abc", 3), 1);
	EQUALS(memcmp("abc", "abc", 3), 0);

	EQUALS_STR(memcchr("abc", 'b', 3), "bc");
	EQUALS(memcchr("abc", 'z', 3), NULL);

	u8 buf[42] = { 0 };
	memset(buf, 42, sizeof(buf));
	memset(buf, 25, 0);
	CHECK(buf[0] == 42 && !memcmp(buf, buf + 1, sizeof(buf) - 1));

	const char *new = "lalala ich bin schlau";
	memcpy(buf + 1, new, strlen(new) + 1);
	EQUALS(buf[0], 42);
	EQUALS_STR(buf + 1, new);
}

TEST(print)
{
	char buf[128] = { 0 };
	snprintf(buf, sizeof(buf), "%% %c %s %d %x %f", 'a', "awesome", 4242, 6942, 0.123456789);
	EQUALS_STR(buf, "% a awesome 4242 1b1e 0.12345");
}

TEST(stack)
{
	int data0, data1;

	struct stack *stack = stack_new();
	CHECK(!!stack);
	EQUALS(stack->tail, NULL);
	stack_push(stack, &data0);
	stack_push_bot(stack, &data1);
	CHECK(!stack_empty(stack));
	EQUALS(stack_peek(stack), &data0);
	EQUALS(stack_pop(stack), &data0);
	EQUALS(stack_peek(stack), &data1);
	stack_clear(stack);
	CHECK(stack_empty(stack));
	stack_destroy(stack);
}

TEST(str)
{
	EQUALS(strlen(""), 0);
	EQUALS(strlen("melvix"), 6);
	EQUALS(strlen("huhu\0abc"), 4);
	EQUALS(strnlen("melvix", 3), 3);
	EQUALS(strnlen("melvix", 0), 0);

	EQUALS_STR(strcchr("awesome", 's'), "some");
	EQUALS_STR(strcchr("awesome", 'e'), "esome");
	EQUALS(strcchr("awesome", 'z'), NULL);
	EQUALS_STR(strcchr("awesome", 's'), "some");
	EQUALS_STR(strrcchr("awesome", 'e'), "e");
	EQUALS(strrcchr("awesome", 'z'), NULL);

	EQUALS(strcmp("aaa", "aaz"), -25);
	EQUALS(strcmp("aaa", "aab"), -1);
	EQUALS(strcmp("aaa", "aaa"), 0);
	EQUALS(strcmp("aaz", "aaa"), 25);
	EQUALS(strcmp("aab", "aaa"), 1);

	char *inv = strdup("melvin");
	EQUALS_STR(strinv(inv), "nivlem");
	free(inv);
	inv = strdup("");
	EQUALS_STR(strinv(inv), "");
	free(inv);

	char buf[42] = { 0 };
	EQUALS(strlcpy(buf, "apple", sizeof(buf)), 5);
	EQUALS_STR(buf, "apple");
	EQUALS(strlcpy(buf, "banana", 4), 6);
	EQUALS_STR(buf, "ban");
	EQUALS(strlcat(buf, "elele", sizeof(buf)), 8);
	EQUALS_STR(buf, "banelele");
}

int main(void)
{
	EXEC(conv);
	EXEC(crypto);
	EXEC(def);
	EXEC(list);
	EXEC(math);
	EXEC(mem);
	EXEC(print);
	EXEC(stack);
	EXEC(str);

	/* fuzz(); */

	if (failed)
		log("[FAIL] %d tests failed\n", failed);
	else
		log("All tests passed\n");

	boot(SYS_BOOT_SHUTDOWN);

	return 0;
}
