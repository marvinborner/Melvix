#include <stddef.h>
#include <stdint.h>
#include <kernel/paging/paging.h>
#include <kernel/io/io.h>

void *memcpy(void *dest, const void *src, size_t count) {
    const char *sp = (const char *) src;
    char *dp = (char *) dest;
    for (; count != 0; count--) *dp++ = *sp++;
    return dest;
}

void *memset(void *dest, char val, size_t count) {
    char *temp = (char *) dest;
    for (; count != 0; count--) *temp++ = val;
    return dest;
}

int memcmp(const void *a_ptr, const void *b_ptr, size_t size) {
    const unsigned char *a = (const unsigned char *) a_ptr;
    const unsigned char *b = (const unsigned char *) b_ptr;
    for (size_t i = 0; i < size; i++) {
        if (a[i] < b[i])
            return -1;
        else if (b[i] < a[i])
            return 1;
    }
    return 0;
}

uint32_t total_memory;

struct memory_entry {
    uint64_t base;
    uint64_t length;
    uint32_t type;
} __attribute__((packed));

struct memory_entry *memory_get_entries() {
    return (struct memory_entry *) 0xA000;
}

void memory_init() {
    uint64_t maxbase = 0;
    uint64_t maxlength = 0;
    for (struct memory_entry *i = memory_get_entries(); i->type; i++) {
        if (i->type == 1 && i->base > maxbase) {
            maxbase = i->base;
            maxlength = i->length;
        }
    }
    total_memory = maxbase + maxlength;
    total_memory /= 1024;
    serial_write_dec(total_memory);
}

uint32_t memory_get_free() {
    return (total_memory) - paging_get_used_pages() * 4;
}

uint32_t memory_get_all() { return total_memory; }