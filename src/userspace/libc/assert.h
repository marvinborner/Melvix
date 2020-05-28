#ifndef MELVIX_ASSERT_H
#define MELVIX_ASSERT_H

#define __FILENAME__                                                                               \
	(__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#define assert(exp) (exp) ? 0 : _assert(__FILENAME__, __LINE__, __func__, #exp)

#endif