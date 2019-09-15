#include "../../io/io.h"
#include "../../interrupts/interrupts.h"
#include "../../graphics/vga.h"

char mouse_cycle = 0;
signed char mouse_byte[3], mouse_ex[3];
signed char mouse_x = 0;
signed char mouse_y = 0;
int mouse_but_1 = 0;
int mouse_but_2 = 0;
int mm_n[3] = {0, 0, 0,};

void mouse_handler(struct regs *a_r) {
    switch (mouse_cycle) {
        case 0:
            mouse_byte[0] = receive(0x60);
            mouse_cycle++;
            break;
        case 1:
            mouse_byte[1] = receive(0x60);
            mouse_cycle++;
            break;
        case 2:
            mouse_byte[2] = receive(0x60);
            mouse_x = mouse_byte[1];
            mouse_y = mouse_byte[2];
            mouse_but_1 = (mouse_byte[0] % 2);
            mouse_but_2 = ((mouse_byte[0] % 4) - (mouse_byte[0] % 2)) / 2;
            mouse_cycle = 0;
            mouse_ex[0] = mouse_byte[0];
            mm_n[0] = 1;
            mouse_ex[1] = mouse_byte[1];
            mm_n[1] = 1;
            mouse_ex[2] = mouse_byte[2];
            mm_n[2] = 1;
            break;
        default:
            break;
    }
    if (mm_n[0] == 1) {
        terminal_write_string("CLICK!\n");
    }
}

inline void mouse_wait(char a_type) {
    unsigned int _time_out = 100000;
    if (a_type == 0) {
        while (_time_out--) {
            if ((receive(0x64) & 1) == 1) {
                return;
            }
        }
        return;
    } else {
        while (_time_out--) {
            if ((receive(0x64) & 2) == 0) {
                return;
            }
        }
        return;
    }
}

inline void mouse_write(char a_write) {
    mouse_wait(1);
    send(0x64, 0xD4);
    mouse_wait(1);
    send(0x60, a_write);
}

char mouse_read() {
    mouse_wait(0);
    return receive(0x60);
}

void mouse_install() {
    char _status;

    // Enable auxiliary mouse device
    mouse_wait(1);
    send(0x64, 0xA8);

    // Enable interrupts
    mouse_wait(1);
    send(0x64, 0x20);
    mouse_wait(0);
    _status = (receive(0x60) | 2);
    mouse_wait(1);
    send(0x64, 0x60);
    mouse_wait(1);
    send(0x60, _status);

    // Use default settings
    mouse_write(0xF6);
    mouse_read();

    // Enable mouse
    mouse_write(0xF4);
    mouse_read();

    // Setup the mouse handler
    irq_install_handler(2, mouse_handler);
}


char get_mouse(int n) {
    if (mm_n[n] == 1) {
        mm_n[n] = 0;
        return mouse_ex[n];
    } else
        return 0;
}