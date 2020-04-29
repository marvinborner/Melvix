#include <syscall.h>
#include <gui.h>

void main()
{
	gui_init();
	syscall_exec("/bin/sh");

	while (1) {
	};
}