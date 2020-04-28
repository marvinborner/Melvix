#include <syscall.h>
#include <mlibc/stdlib.h>

void main()
{
	syscall_halt();
	// As char[]:       0xC105BFD6
	// As const char *: 0x8048B20

	/* char test[] = "banane"; */
	/* syscall_write(test); */
	/* syscall_halt(); */
}