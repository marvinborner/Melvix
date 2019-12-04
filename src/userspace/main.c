#include <syscall.h>

void user_main()
{
    const char hello[] = "> Successfully to usermode!\n";
    syscall_write(0, hello, sizeof(hello));

    while (1) {};
}