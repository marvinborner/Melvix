#include <syscall.h>

void writec(char c)
{
    syscall_writec(c);
}