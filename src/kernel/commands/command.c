#include <kernel/lib/lib.h>
#include <kernel/io/io.h>
#include <kernel/acpi/acpi.h>
#include <kernel/graphics/vesa.h>
#include <kernel/cmos/rtc.h>
#include <kernel/timer/timer.h>
#include <kernel/lib/string.h>
#include <kernel/lib/stdio.h>

int32_t starts_with(const char *a, const char *b)
{
    size_t length_pre = strlen(b);
    size_t length_main = strlen(a);
    return length_main < length_pre ? 0 : memcmp(b, a, length_pre) == 0;
}

void exec_command(char *command)
{
    if (starts_with(command, "ls"))
        printf("Listing files\n");
    else if (starts_with(command, "help"))
        printf("I can't help you write now\n");
    else if (starts_with(command, "ping"))
        printf("pong!\n");
    else if (starts_with(command, "clear"))
        vesa_clear();
    else if (starts_with(command, "shutdown") || starts_with(command, "exit"))
        acpi_poweroff();
    else if (starts_with(command, "zzz"))
        printf("Not implemented\n");
    else if (starts_with(command, "time"))
        printf("%d\n", (int) get_time());
    else if (starts_with(command, "date"))
        write_time();
    else if (starts_with(command, "reboot"))
        reboot();
    else if (command[0] != 0)
        warn("Command not found!");
}
