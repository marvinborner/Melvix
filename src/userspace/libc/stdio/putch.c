#include <syscall.h>

void putch(char ch)
{
	if (ch != 0)
		syscall_putch(ch);
}