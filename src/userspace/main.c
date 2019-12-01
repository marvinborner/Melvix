#include <stddef.h>

extern void syscall();

void user_main()
{
    syscall();

    while (1) {};
}