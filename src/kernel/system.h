#ifndef MELVIX_SYSTEM_H
#define MELVIX_SYSTEM_H

#include "graphics/graphics.h"

/**
 * Initialize the basic features of the OS
 */
void init();

/**
 * The ASM registers as packed structure
 */
typedef struct __attribute__ ((packed)) {
    unsigned short di, si, bp, sp, bx, dx, cx, ax;
    unsigned short gs, fs, es, ds, eflags;
} regs16_t;

/**
 * Execute BIOS interrupts by switching to real mode
 * @param intnum The interrupt number (e.g. 0x10)
 * @param regs The ASM registers
 */
extern void int32(unsigned char intnum, regs16_t *regs);

/**
 * ASM segment:offset pointer
 */
struct far_ptr {
    union {
        uint32_t ptr;
        struct {
            uint16_t offset, segment;
        };
    };
} __attribute__ ((packed));

typedef struct far_ptr far_ptr_t;

/**
 * Get offset from ASM segment:offset pointer
 */
uint16_t get_offset(const volatile void *p) {
    return (uint16_t) (uintptr_t) p & 0x000F;
}

/**
 * Get segment from ASM segment:offset pointer
 */
uint16_t get_segment(const volatile void *p) {
    return (uint16_t) (((uintptr_t) p) >> 4);
}

/**
 * Convert pointer to far_ptr
 * @param __ptr The ASM segment:offset pointer
 * @return The new far pointer
 */
far_ptr_t FAR_PTR(void *__ptr) {
    far_ptr_t __fptr;
    __fptr.offset = get_offset(__ptr);
    __fptr.segment = get_segment(__ptr);
    return __fptr;
}

/**
 * Get pointer from ASM segment:offset far pointer
 * @param fptr The ASM far pointer
 * @return The normalized pointer
 */
void *get_ptr(far_ptr_t fptr) {
    return (void *) (unsigned long) ((fptr.segment << 4) + fptr.offset);
}

/**
 * Display a warning message
 * TODO: Add line number and file name
 * @param msg The warning cause/reason
 */
void warn(char *msg) {
    asm volatile ("cli");
    terminal_set_color(6);
    terminal_write_line("WARNING");
    terminal_write_string(msg);
    terminal_set_color(7);
}

/**
 * Halt the entire system and display a message
 * TODO: Add line number and file name
 * @param msg The error cause/reason
 */
void panic(char *msg) {
    asm volatile ("cli");
    terminal_set_color(4);
    terminal_write_line("PANIC");
    terminal_write_string(msg);
    loop:
    asm volatile ("hlt");
    goto loop;
}

/**
 * Assert that a value is non-zero, else panic
 * TODO: Add line number and file name
 * @param x The value
 */
void assert(int x) {
    if (x == 0) {
        panic("Assertion failed");
    }
}

#endif
