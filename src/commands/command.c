#include "../graphics/graphics.h"
#include "../lib/lib.h"
#include "../io/io.h"

int32_t starts_with(const char *a, const char *b) {
    size_t length_pre = strlen(b);
    size_t length_main = strlen(a);
    return length_main < length_pre ? 0 : memory_compare(b, a, length_pre) == 0;
}

extern void shutdown();

void exec_command(char *command) {
    if (starts_with(command, "ls"))
        terminal_write_line("Listing files");
    else if (starts_with(command, "help"))
        terminal_write_line("I can't help you write now");
    else if (starts_with(command, "ping"))
        terminal_write_line("pong!");
    else if (starts_with(command, "shutdown"))
        shutdown();
    else
        terminal_write_line("Command not found!");
}
