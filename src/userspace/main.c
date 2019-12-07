#include <syscall.h>
#include <mlibc/string.h>

void user_main()
{
    char hello[] = "> Successfully switched to usermode!\n";
    syscall_write(hello);

    while (1) {
        char *command = (char *) syscall_read();
        char test[1024];
        strcpy(test, command);
        syscall_write(command);
    };
}