#ifndef MELVIX_STDLIB_H
#define MELVIX_STDLIB_H

#include <stdint.h>

#ifndef MELVIX_STRING_H

#include <kernel/lib/string.h>

#endif

char *itoa(int n);

int atoi(char *str);

char *htoa(uint32_t n);

int htoi(char *str);

#endif