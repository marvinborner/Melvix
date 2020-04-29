#ifndef MELVIX_STDLIB_H
#define MELVIX_STDLIB_H

#include <stdint.h>

// String conversion
char *itoa(int n);
int atoi(char *str);
char *htoa(u32 n);
int htoi(char *str);

// Exit functions

// Memory management
void *malloc(u8 size);
void free(void *addr);

#endif