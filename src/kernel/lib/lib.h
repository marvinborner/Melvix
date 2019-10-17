#ifndef MELVIX_LIB_H
#define MELVIX_LIB_H

#include <stddef.h>

/**
 * Find the length of a string
 * @param str The string pointer which size should be calculated
 * @return The length of str
 */
size_t strlen(const char *str);

/**
 * Compare two strings
 * @param s1 The first string pointer
 * @param s2 The second string pointer
 * @return The length difference between s1 and s2
 */
size_t strcmp(const char *s1, const char *s2);

/**
 * Append the data of src to dest
 * @param dest The string destination pointer
 * @param src The string pointer that will get appended
 */
void strcat(char *dest, const char *src);

/**
 * Copy the data of src to dest
 * @param dest The copying destination pointer (gets replaced)
 * @param src The string pointer that will get copied
 */
void strcpy(char *dest, const char *src);

/**
 * Copy n data from src to dest
 * @param dest The destination array pointer
 * @param src The source array pointer of the data
 * @param count The number of bytes to be copied (src)
 * @return The modified dest pointer
 */
void *memory_copy(void *dest, const void *src, size_t count);

/**
 * Replace n bytes of dest by val
 * @param dest The destination array pointer
 * @param val The replacing chracater
 * @param count The number of times val should replace dest entry
 * @return The modified dest pointer
 */
void *memory_set(void *dest, char val, size_t count);

/**
 * Compare the first n bytes of a and b
 * @param a_ptr The first memory area pointer
 * @param b_ptr The second memory area pointer
 * @param size The number of bytes to be compared
 * @return -1 if a < b, 0 if a = b and 1 if a > b
 */
int memory_compare(const void *a_ptr, const void *b_ptr, size_t size);

#endif
