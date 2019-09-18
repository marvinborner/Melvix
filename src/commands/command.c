#include "../graphics/graphics.h"
#include "../lib/lib.h"

int32_t starts_with(const char *a, const char *b) {
    if (strcmp(a, b, strlen(b)) == 0)
        return 1
    return 0
}

void exec_command(char *command) {
    if (starts_with(command, "ls"))
        terminal_write_string("// listing files");
    else if (starts_with(command, "help"))
	terminal_write_string("I can't help you write now");
    else if (starts_with(command, "ping"))
	terminal_write_string("pong!")
}
