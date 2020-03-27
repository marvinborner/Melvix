#ifndef MELVIX_LIB_H
#define MELVIX_LIB_H

#include <stddef.h>
#include <stdint.h>

/**
 * Copy n data from src to dest
 * @param dest The destination array pointer
 * @param src The source array pointer of the data
 * @param count The number of bytes to be copied (src)
 * @return The modified dest pointer
 */
void *memcpy(void *dest, const void *src, size_t count);

/**
 * Replace n bytes of dest by val
 * @param dest The destination array pointer
 * @param val The replacing chracater
 * @param count The number of times val should replace dest entry
 * @return The modified dest pointer
 */
void *memset(void *dest, char val, size_t count);

/**
 * Compare the first n bytes of a and b
 * @param a_ptr The first memory area pointer
 * @param b_ptr The second memory area pointer
 * @param size The number of bytes to be compared
 * @return -1 if a < b, 0 if a = b and 1 if a > b
 */
int memcmp(const void *a_ptr, const void *b_ptr, size_t size);

void memory_init();

uint32_t memory_get_all();

#endif