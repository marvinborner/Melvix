#ifndef MELVIX_GDT_H
#define MELVIX_GDT_H

#include <stdint.h>

/**
 * Installs the Global Descriptor Table
 */
void gdt_install();

void tss_write(s32 num, u16 ss0, u32 esp0);

void tss_flush();

void set_kernel_stack(u32 stack);

#endif