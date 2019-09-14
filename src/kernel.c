#include "graphics/vga.h"

void kernel_main(void) {
    terminal_initialize();
    terminal_write_string("Melvix loaded successfully!\nTest");
}