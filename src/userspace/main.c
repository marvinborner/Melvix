#include <syscall.h>

void user_main()
{
    char hello[] = "> Successfully switched to usermode!\n";
    syscall_write(hello);

    while (1) {
        char buffer[20] = {'\0'};
        syscall_read(buffer);
        syscall_write(buffer);
    };
}