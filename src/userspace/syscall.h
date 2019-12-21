#ifndef MELVIX_SYSCALL_H
#define MELVIX_SYSCALL_H

#include <stdint.h>

#define DECL_SYSCALL0(fn) int syscall_##fn()
#define DECL_SYSCALL1(fn, p1) int syscall_##fn(p1)
#define DECL_SYSCALL2(fn, p1, p2) int syscall_##fn(p1,p2)
#define DECL_SYSCALL3(fn, p1, p2, p3) int syscall_##fn(p1,p2,p3)
#define DECL_SYSCALL4(fn, p1, p2, p3, p4) int syscall_##fn(p1,p2,p3,p4)
#define DECL_SYSCALL5(fn, p1, p2, p3, p4, p5) int syscall_##fn(p1,p2,p3,p4,p5)

#define DEFN_SYSCALL0(fn, num) \
    int syscall_##fn() { \
        int a; __asm__ __volatile__("int $0x80" : "=a" (a) : "0" (num)); \
        return a; \
    }

#define DEFN_SYSCALL1(fn, num, P1) \
    int syscall_##fn(P1 p1) { \
        int __res; __asm__ __volatile__("push %%ebx; movl %2,%%ebx; int $0x80; pop %%ebx" \
                : "=a" (__res) \
                : "0" (num), "r" ((int)(p1)) \
                : "memory"); \
        return __res; \
    }

#define DEFN_SYSCALL2(fn, num, P1, P2) \
    int syscall_##fn(P1 p1, P2 p2) { \
        int __res; __asm__ __volatile__("push %%ebx; movl %2,%%ebx; int $0x80; pop %%ebx" \
                : "=a" (__res) \
                : "0" (num), "r" ((int)(p1)), "c"((int)(p2)) \
                : "memory"); \
        return __res; \
    }

#define DEFN_SYSCALL3(fn, num, P1, P2, P3) \
    int syscall_##fn(P1 p1, P2 p2, P3 p3) { \
        int __res; __asm__ __volatile__("push %%ebx; movl %2,%%ebx; int $0x80; pop %%ebx" \
                : "=a" (__res) \
                : "0" (num), "b" ((int)(p1)), "c"((p2)), "d"((int)(p3)) \
                : "memory"); \
        return __res; \
    }

#define DEFN_SYSCALL4(fn, num, P1, P2, P3, P4) \
    int syscall_##fn(P1 p1, P2 p2, P3 p3, P4 p4) { \
        int __res; __asm__ __volatile__("push %%ebx; movl %2,%%ebx; int $0x80; pop %%ebx" \
                : "=a" (__res) \
                : "0" (num), "b" ((int)(p1)), "c"((int)(p2)), "d"((int)(p3)), "S"((int)(p4)) \
                : "memory"); \
        return __res; \
    }

#define DEFN_SYSCALL5(fn, num, P1, P2, P3, P4, P5) \
    int syscall_##fn(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) { \
        int __res; __asm__ __volatile__("push %%ebx; movl %2,%%ebx; int $0x80; pop %%ebx" \
                : "=a" (__res) \
                : "0" (num), "b" ((int)(p1)), "c"((int)(p2)), "d"((int)(p3)), "S"((int)(p4)), "D"((int)(p5)) \
                : "memory"); \
        return __res; \
    }

/**
 * DECLARATIONS
 */
DECL_SYSCALL0(halt);

DECL_SYSCALL1(write, char *);

DECL_SYSCALL1(read, char *);

DECL_SYSCALL1(writec, char);

DECL_SYSCALL0(readc);

DECL_SYSCALL0(get_pointers);

DECL_SYSCALL1(paging_alloc, uint32_t);

DECL_SYSCALL2(paging_free, uint32_t, uint32_t);

#endif
