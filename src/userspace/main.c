#include <syscall.h>
#include <mlibc/stdlib.h>

int32_t starts_with(const char *a, const char *b)
{
	size_t length_pre = strlen(b);
	size_t length_main = strlen(a);
	return length_main < length_pre ? 0 : memcmp(b, a, length_pre) == 0;
}

void main()
{
	syscall_halt();
	// As char[]:       0xC105BFD6
	// As const char *: 0x8048B20

	char test[] = "banane";
	syscall_write(test);
	syscall_halt();
}