#ifndef MELVIX_IO_H
#define MELVIX_IO_H

#include <stdint.h>

uint8_t receive_b(uint16_t port);

uint16_t receive_w(uint16_t port);

uint32_t receive_l(uint16_t port);

void send_b(uint16_t port, uint8_t data);

void send_w(uint16_t port, uint16_t data);

void send_l(uint16_t port, uint32_t data);

#endif
