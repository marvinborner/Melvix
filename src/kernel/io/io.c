#include <stdint.h>
#include "../lib/lib.h"

uint8_t receive_b(uint16_t port) {
    unsigned char value;
    asm volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

uint16_t receive_w(uint16_t port) {
    unsigned char value;
    asm volatile("inb %1,%0" : "=a"(value) : "Nd"(port)); // TODO: Fix inw error
    return value;
}

uint32_t receive_l(uint16_t port) {
    unsigned char value;
    asm volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void send_b(uint16_t port, uint8_t data) {
    asm volatile ("outb %0, %1"::"a" (data), "Nd"(port));
}

void send_w(uint16_t port, uint16_t data) {
    asm volatile ("outw %0, %1"::"a" (data), "Nd"(port));
}

void send_l(uint16_t port, uint32_t data) {
    asm volatile ("outl %0, %1"::"a" (data), "Nd"(port));
}

void init_serial() {
    send_b(0x3f8 + 1, 0x00);
    send_b(0x3f8 + 3, 0x80);
    send_b(0x3f8 + 0, 0x03);
    send_b(0x3f8 + 1, 0x00);
    send_b(0x3f8 + 3, 0x03);
    send_b(0x3f8 + 2, 0xC7);
    send_b(0x3f8 + 4, 0x0B);
}

int is_transmit_empty() {
    return receive_b(0x3f8 + 5) & 0x20;
}

void write_serial(char *data) {
    for (size_t i = 0; i < strlen(data); i++) {
        while (is_transmit_empty() == 0);
        send_b(0x3f8, data[i]);
    }
}
