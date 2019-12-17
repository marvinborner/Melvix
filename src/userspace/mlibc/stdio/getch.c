#include <syscall.h>

char getch()
{
    return (char) syscall_readc();
}