#include <kernel/input/input.h>
#include <kernel/lib/string.h>

char *readline()
{
	keyboard_clear_buffer();
	while (keyboard_buffer[strlen(keyboard_buffer) - 1] != '\n') {
	}
	return keyboard_buffer;
}