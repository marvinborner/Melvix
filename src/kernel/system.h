#ifndef MELVIX_SYSTEM_H
#define MELVIX_SYSTEM_H

#include <stdint.h>
#include "timer/timer.h"
#include "lib/lib.h"

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

/**
 * Get offset from ASM segment:offset pointer
 */
static inline uint16_t get_offset(const volatile void *p) {
    return (uint16_t) (uintptr_t) p & 0x000F;
}

/**
 * Get segment from ASM segment:offset pointer
 */
static inline uint16_t get_segment(const volatile void *p) {
    return (uint16_t) (((uintptr_t) p) >> 4);
}

/**
 * Convert pointer to far_ptr
 * @param __ptr The ASM segment:offset pointer
 * @return The new far pointer
 */
static inline struct far_ptr FAR_PTR(void *__ptr) {
    struct far_ptr __fptr;
    __fptr.offset = get_offset(__ptr);
    __fptr.segment = get_segment(__ptr);
    return __fptr;
}

/**
 * Get pointer from ASM segment:offset far pointer
 * @param fptr The ASM far pointer
 * @return The normalized pointer
 */
static inline void *get_ptr(struct far_ptr fptr) {
    return (void *) (unsigned long) ((fptr.segment << 4) + fptr.offset);
}

/**
 * Print the current kernel time
 */
static inline void kernel_time() {
    terminal_write_string("\n");
    terminal_write_string("[");
    terminal_write_number(get_time());
    terminal_write_string("] ");
}

/**
 * Display an information message
 * @param msg The information
 */
static inline void info(char *msg) {
    terminal_set_color(9);
    kernel_time();
    terminal_write_string("INFO: ");
    terminal_write_string(msg);
    terminal_write_string("\r");
    terminal_set_color(7);
}

/**
 * Display a warning message
 * TODO: Add line number and file name
 * @param msg The warning cause/reason
 */
static inline void warn(char *msg) {
    terminal_set_color(6);
    kernel_time();
    terminal_write_string("WARNING: ");
    terminal_write_string(msg);
    terminal_write_string("\r");
    terminal_set_color(7);
}

/**
 * Halt the entire system and display a message
 * TODO: Add line number and file name
 * @param msg The error cause/reason
 */
static inline void panic(char *msg) {
    asm volatile ("cli");
    terminal_set_color(4);
    kernel_time();
    terminal_write_string("PANIC: ");
    terminal_write_string(msg);
    terminal_write_string(" - System Halted!");
    write_serial(msg);
    loop:
    asm volatile ("hlt");
    goto loop;
}

/**
 * Assert that a value is non-zero, else panic
 * TODO: Add line number and file name
 * @param x The value
 */
static inline void assert(int x) {
    if (x == 0) {
        panic("Assertion failed");
    }
}

#endif
