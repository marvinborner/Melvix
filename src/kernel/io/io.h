#ifndef MELVIX_IO_H
#define MELVIX_IO_H

#include <stdint.h>

/**
 * Receive from specified hardware port
 * @param port The hardware port
 * @return The hardware response
 */
uint8_t receive_b(uint16_t port);

/**
 * Receive from specified hardware port
 * @param port The hardware port
 * @return The hardware response
 */
uint16_t receive_w(uint16_t port);

/**
 * Receive from specified hardware port
 * @param port The hardware port
 * @return The hardware response
 */
uint32_t receive_l(uint16_t port);

/**
 * Send data to the specified hardware port
 * @param port The hardware port
 * @param data The data that should be sent
 */
void send_b(uint16_t port, uint8_t data);

/**
 * Send data to the specified hardware port
 * @param port The hardware port
 * @param data The data that should be sent
 */
void send_w(uint16_t port, uint16_t data);

/**
 * Send data to the specified hardware port
 * @param port The hardware port
 * @param data The data that should be sent
 */
void send_l(uint16_t port, uint32_t data);

#endif
