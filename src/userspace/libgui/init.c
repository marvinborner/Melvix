#include <stdint.h>
#include <syscall.h>
#include <gui.h>

struct pointers *pointers;

void gui_init()
{
	pointers = syscall_pointers();
}