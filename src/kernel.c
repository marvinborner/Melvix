#include "graphics/graphics.h"
#include "gdt/gdt.h"
#include "interrupts/interrupts.h"
#include "input/input.h"
#include "timer/timer.h"
#include "sound/sound.h"

void kernel_main(void) {
    gdt_install();
    idt_install();
    isrs_install();
    irq_install();

    __asm__ __volatile__ ("sti");

    terminal_initialize();
    timer_install();
    keyboard_install();
    mouse_install();

    terminal_write_string("Melvix loaded successfully!\n");

    beep(262, 20);
    beep(294, 20);
    beep(330, 20);
    beep(349, 20);
    beep(392, 20);
    beep(440, 20);
    beep(494, 20);
    beep(523, 20);
    // __asm__  ("div %0" :: "r"(0)); // Exception testing x/0
}