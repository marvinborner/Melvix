#ifndef MELVIX_STDLIB_H
#define MELVIX_STDLIB_H

#include <stdint.h>

// String conversion
char *itoa(int n);
int atoi(char *str);
char *htoa(u32 n);
int htoi(char *str);

// Exit functions
void exit(u32 code);
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

// Memory management
void *malloc(u32 size);
void free(void *addr);

#endif