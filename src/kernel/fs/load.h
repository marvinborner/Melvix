#ifndef MELVIX_LOAD_H
#define MELVIX_LOAD_H

#include <stdint.h>

uint32_t userspace;

struct font *font;

struct font {
    uint16_t font_32[758][32];
    uint16_t font_24[758][24];
    uint8_t font_16[758][16];
    uint16_t cursor[19];
};

void load_binaries();

#endif
