#ifndef MELVIX_IO_H
#define MELVIX_IO_H

#include <stdint.h>

/**
 * Receive from specified hardware port
 * @param port The hardware port
 * @return The hardware response
 */
uint8_t inb(uint16_t port);

/**
 * Receive from specified hardware port
 * @param port The hardware port
 * @return The hardware response
 */
uint16_t inw(uint16_t port);

/**
 * Receive from specified hardware port
 * @param port The hardware port
 * @return The hardware response
 */
uint32_t inl(uint16_t port);

void cli();
void sti();
void hlt();

/**
 * Send data to the specified hardware port
 * @param port The hardware port
 * @param data The data that should be sent
 */
void outb(uint16_t port, uint8_t data);

/**
 * Send data to the specified hardware port
 * @param port The hardware port
 * @param data The data that should be sent
 */
void outw(uint16_t port, uint16_t data);

/**
 * Send data to the specified hardware port
 * @param port The hardware port
 * @param data The data that should be sent
 */
void outl(uint16_t port, uint32_t data);

/**
 * Initialize the serial conenction
 */
void init_serial();

/**
 * Write a single char to the serial port (QEMU logging)
 * @param ch The char
 */
void serial_put(char ch);

#endif
