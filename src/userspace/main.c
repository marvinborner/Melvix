#include <syscall.h>

void user_main()
{
    char hello[] = "> Successfully switched to usermode!\n";
    syscall_write(hello);

    while (1) {
        char ch = (char) syscall_readc();
        syscall_writec(ch);
    };
}