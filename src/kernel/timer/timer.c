#include <kernel/interrupts/interrupts.h>
#include <kernel/io/io.h>
#include <kernel/system.h>

static unsigned int timer_ticks = 0;

void timer_phase(int hz) {
    int divisor = (int) (3579545.0 / 3.0 / (double) hz);
    send_b(0x43, 0x36); // 01 10 11 0b // CTR, RW, MODE, BCD
    send_b(0x40, divisor & 0xFF);
    send_b(0x40, divisor >> 8);
}

// Executed 1000 times per second
void timer_handler(struct regs *r) {
    timer_ticks++;
}

// "Delay" function with CPU sleep
void timer_wait(int ticks) {
    unsigned int eticks;

    eticks = timer_ticks + ticks;
    while (timer_ticks < eticks) {
        asm volatile ("sti//hlt//cli");
    }
}

unsigned int get_time() {
    return timer_ticks;
}

// Install timer handler into IRQ0
void timer_install() {
    timer_phase(1000);
    irq_install_handler(0, timer_handler);
    vga_log("Installed timer", 1);
}