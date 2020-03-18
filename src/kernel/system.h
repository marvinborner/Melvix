#ifndef MELVIX_SYSTEM_H
#define MELVIX_SYSTEM_H

/**
 * The kernel end
 */
extern void ASM_KERNEL_END();

/**
 * The initial stack pointer
 */
uint32_t initial_esp;

/**
 * Initialize the basic features of the OS
 */
void init();

/**
 * The ASM registers as packed structure
 */
typedef struct __attribute__((packed)) {
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
 */
void vga_log(char *msg);

/**
 * Print the current kernel time
 */
void kernel_time();

/**
 * Display a general log message
 * @param msg The message
 */
void debug(const char *fmt, ...);

/**
 * Display an information message
 * @param msg The information
 */
void info(const char *fmt, ...);

/**
 * Display a warning message
 * TODO: Add line number and file name
 * @param msg The warning cause/reason
 */
void warn(const char *fmt, ...);

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

/**
 * Creates an infinite halt loop
 */
void halt_loop();

/**
 * Executes int32 with paging disable/enable
 * @param code The interrupt code
 * @param regs The registers
 */
void v86(uint8_t code, regs16_t *regs);

#endif
