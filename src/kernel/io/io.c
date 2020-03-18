#include <stdint.h>
#include <kernel/system.h>

uint8_t inb(uint16_t port)
{
    uint8_t value;
    asm ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

uint16_t inw(uint16_t port)
{
    uint16_t value;
    asm ("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

uint32_t inl(uint16_t port)
{
    uint32_t value;
    asm ("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void cli()
{
    asm volatile ("cli");
}

void sti()
{
    asm volatile ("sti");
}

void hlt()
{
    asm volatile ("hlt");
}

void outb(uint16_t port, uint8_t data)
{
    asm ("outb %0, %1"::"a" (data), "Nd"(port));
}

void outw(uint16_t port, uint16_t data)
{
    asm ("outw %0, %1"::"a" (data), "Nd"(port));
}

void outl(uint16_t port, uint32_t data)
{
    asm ("outl %0, %1"::"a" (data), "Nd"(port));
}

void init_serial()
{
    outb(0x3f8 + 1, 0x00);
    outb(0x3f8 + 3, 0x80);
    outb(0x3f8 + 0, 0x03);
    outb(0x3f8 + 1, 0x00);
    outb(0x3f8 + 3, 0x03);
    outb(0x3f8 + 2, 0xC7);
    outb(0x3f8 + 4, 0x0B);
    vga_log("Installed serial connection");
}

int is_transmit_empty()
{
    return inb(0x3f8 + 5) & 0x20;
}

void serial_put(char ch)
{
    while (is_transmit_empty() == 0);
    outb(0x3f8, (uint8_t) ch);
}
