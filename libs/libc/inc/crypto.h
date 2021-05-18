// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef CRYPTO_H
#define CRYPTO_H

#include <def.h>

void md5(const void *initial_msg, u32 initial_len, u8 digest[16]) NONNULL;
u32 crc32(u32 crc, const void *buf, u32 size) NONNULL;

#ifdef KERNEL
INLINE u32 crc32_user(u32 crc, const void *buf, u32 size) NONNULL;
#endif

#endif
