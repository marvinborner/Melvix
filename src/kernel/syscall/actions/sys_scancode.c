#include <stdint.h>
#include <kernel/input/input.h>
#include <kernel/io/io.h>

uint32_t sys_scancode()
{
	sti();
	uint32_t key = wait_scancode();
	cli();
	return key;
}