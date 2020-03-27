#ifndef MELVIX_GDT_H
#define MELVIX_GDT_H

/**
 * Installs the Global Descriptor Table
 */
void gdt_install();

void tss_write(int32_t num, uint16_t ss0, uint32_t esp0);

void tss_flush();

void set_kernel_stack(uintptr_t stack);

#endif