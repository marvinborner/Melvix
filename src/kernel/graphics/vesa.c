#include "graphics.h"

extern char *find_mode();

void vesa_init() {
    terminal_write_line(find_mode());
}