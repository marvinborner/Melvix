#include <kernel/lib/lib.h>
#include <kernel/io/io.h>
#include <kernel/acpi/acpi.h>
#include <kernel/graphics/vesa.h>
#include <kernel/cmos/rtc.h>
#include <kernel/timer/timer.h>

int32_t starts_with(const char *a, const char *b) {
    size_t length_pre = strlen(b);
    size_t length_main = strlen(a);
    return length_main < length_pre ? 0 : memory_compare(b, a, length_pre) == 0;
}

void exec_command(char *command) {
    if (strcmp(command, "ls") == 0)
        vesa_draw_string("Listing files\n");
    else if (strcmp(command, "help") == 0)
        vesa_draw_string("I can't help you write now\n");
    else if (strcmp(command, "ping") == 0)
        vesa_draw_string("pong!\n");
    else if (strcmp(command, "clear") == 0)
        vesa_clear();
    else if (strcmp(command, "shutdown") == 0)
        acpi_poweroff();
    else if (strcmp(command, "zzz") == 0)
        vesa_draw_string("Not implemented\n");
    else if (strcmp(command, "time") == 0) {
        vesa_draw_number(get_time());
        vesa_draw_string("\n");
    } else if (strcmp(command, "date") == 0)
        write_time();
    else if (strcmp(command, "reboot") == 0)
        reboot();
    else
        vesa_draw_string("Command not found!\n");
}
