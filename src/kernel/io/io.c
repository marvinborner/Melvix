#include <stdint.h>
#include "../lib/lib.h"
#include "io.h"

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
    serial_write("Installed serial connection!\n");
}

int is_transmit_empty() {
    return receive_b(0x3f8 + 5) & 0x20;
}

void serial_put(char ch) {
    while (is_transmit_empty() == 0);
    send_b(0x3f8, ch);
}

void serial_write(char *data) {
    for (size_t i = 0; i < strlen(data); i++) {
        serial_put(data[i]);
    }
}

void serial_write_hex(int n) {
    int tmp;

    serial_write("0x");
    char noZeroes = 1;

    for (int i = 28; i > 0; i -= 4) {
        tmp = (n >> i) & 0xF;
        if (tmp == 0 && noZeroes != 0) continue;

        if (tmp >= 0xA) {
            noZeroes = 0;
            serial_put(tmp - 0xA + 'a');
        } else {
            noZeroes = 0;
            serial_put(tmp + '0');
        }
    }

    tmp = n & 0xF;
    if (tmp >= 0xA) {
        serial_put(tmp - 0xA + 'a');
    } else {
        serial_put(tmp + '0');
    }
}

void serial_write_dec(int n) {
    if (n == 0) serial_put('0');
    int acc = n;
    char c[32];
    int i = 0;
    while (acc > 0) {
        c[i] = '0' + acc % 10;
        acc /= 10;
        i++;
    }
    c[i] = 0;
    static char c2[32];
    c2[i--] = 0;
    int j = 0;
    while (i >= 0) c2[i--] = c[j++];
    serial_write(c2);
}
