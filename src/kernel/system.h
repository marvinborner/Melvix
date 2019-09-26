#ifndef MELVIX_SYSTEM_H
#define MELVIX_SYSTEM_H

void init();

typedef struct __attribute__ ((packed)) {
    unsigned short di, si, bp, sp, bx, dx, cx, ax;
    unsigned short gs, fs, es, ds, eflags;
} regs16_t;

extern void int32(unsigned char intnum, regs16_t *regs);

struct far_ptr {
    union {
        uint32_t ptr;
        struct {
            uint16_t offset, segment;
        };
    };
} __attribute__ ((packed));

static inline uint16_t get_offset(const volatile void *p) {
    return (uint16_t) (uintptr_t) p & 0x000F;
}

static inline uint16_t get_segment(const volatile void *p) {
    return (uint16_t) (((uintptr_t) p) >> 4);
}

static inline struct far_ptr FAR_PTR(void *__ptr) {
    struct far_ptr __fptr;
    __fptr.offset = get_offset(__ptr);
    __fptr.segment = get_segment(__ptr);
    return __fptr;
}

static inline void *get_ptr(struct far_ptr fptr) {
    return (void *) (unsigned long) ((fptr.segment << 4) + fptr.offset);
}

#endif
