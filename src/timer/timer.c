#include "../interrupts/interrupts.h"
#include "../io/io.h"

volatile unsigned int timer_ticks = 0;

void timer_phase(int hz) {
    int divisor = 1193180 / hz;
    send(0x43, 0x36); // 01 10 11 0b // CTR, RW, MODE, BCD
    send(0x40, divisor & 0xFF);
    send(0x40, divisor >> 8);
}

// Executed 100 times per second
void timer_handler(struct regs *r) {
    timer_ticks++;
}

// "Delay" function with CPU sleep
void timer_wait(int ticks) {
    unsigned int eticks;

    eticks = timer_ticks + ticks;
    while (timer_ticks < eticks) {
        __asm__ __volatile__ ("sti//hlt//cli");
    }
}

// Install timer handler into IRQ0
void timer_install() {
    timer_phase(100);
    irq_install_handler(0, timer_handler);
}