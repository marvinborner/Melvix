#include <kernel/input/input.h>
#include <kernel/timer/timer.h>

char getch()
{
    keyboard_char_buffer = 0;
    while (keyboard_char_buffer == 0) {
        timer_wait(1); // IDK why!
    }
    return keyboard_char_buffer;
}