unsigned char receive(unsigned short port) {
    unsigned char value;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (value) : "dN" (port));
    return value;
}

void send(unsigned short port, unsigned char data) {
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (port), "a" (data));
}
