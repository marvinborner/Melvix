#ifndef MELVIX_SYSTEM_H
#define MELVIX_SYSTEM_H

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

#endif
