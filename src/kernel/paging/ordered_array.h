#ifndef MELVIX_ORDERED_ARRAY_H
#define MELVIX_ORDERED_ARRAY_H

#include <stdint.h>

/**
 * The array can store anything - so a void pointer is used
 */
typedef void *type_t;

typedef char (*lessthan_predicate_t)(type_t, type_t);

typedef struct {
    type_t *array;
    uint32_t size;
    uint32_t max_size;
    lessthan_predicate_t less_than;
} ordered_array_t;

/**
 * A standard less-than predicate
 * @param a The first parameter
 * @param b The second parameter
 * @return Non-zero if the first parameter is bigger than the second
 */
char standard_lessthan_predicate(type_t a, type_t b);

/**
 * Create an ordered array
 * @param max_size Maximal size
 * @param less_than The less-than predicate
 * @return The newly created array
 */
ordered_array_t create_ordered_array(uint32_t max_size, lessthan_predicate_t less_than);

ordered_array_t place_ordered_array(void *addr, uint32_t max_size, lessthan_predicate_t less_than);

/**
 * Destroy an ordered array
 * @param array The ordered array
 */
void destroy_ordered_array(ordered_array_t *array);

/**
 * Add an item into the array
 * @param item The item
 * @param array The array
 */
void insert_ordered_array(type_t item, ordered_array_t *array);

/**
 * Lookup the item at a specific index
 * @param i The index
 * @param array The array
 * @return
 */
type_t lookup_ordered_array(uint32_t i, ordered_array_t *array);

/**
 * Delete an item at an index from an array
 * @param i The index
 * @param array The array
 */
void remove_ordered_array(uint32_t i, ordered_array_t *array);

#endif
