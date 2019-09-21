#include <stdint.h>

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
