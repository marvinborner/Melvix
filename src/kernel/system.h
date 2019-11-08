#ifndef MELVIX_SYSTEM_H
#define MELVIX_SYSTEM_H

/**
 * The kernel end
 */
extern void *ASM_KERNEL_END;

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
 * The vga log buffer to transfer the logs to VESA
 */
char *vga_buffer;

/**
 * Log a message before VESA has been initialized
 * @param msg The message
 * @param line The hardcoded linenumber
 */
void vga_log(char *msg, int line);

/**
 * Print the current kernel time
 */
void kernel_time();

/**
 * Display a general log message
 * @param msg The message
 */
void log(char *msg);

/**
 * Display an information message
 * @param msg The information
 */
void info(char *msg);

/**
 * Display a warning message
 * TODO: Add line number and file name
 * @param msg The warning cause/reason
 */
void warn(char *msg);

/**
 * Halt the entire system and display a message
 * TODO: Add line number and file name
 * @param msg The error cause/reason
 */
void panic(char *msg);

/**
 * Assert that a value is non-zero, else panic
 * TODO: Add line number and file name
 * @param x The value
 */
void assert(int x);

#endif
