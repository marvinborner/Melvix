// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef X86_32_ARCH_H
#define X86_32_ARCH_H

#include <kernel.h>

NORETURN void arch_halt(void);
void arch_log_enable(void);
void arch_log_disable(void);
void arch_log(const char *data, size_t count) NONNULL;

#endif
