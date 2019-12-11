#include <syscall.h>
#include <graphics/graphics.h>

void user_main()
{
    char hello[] = "> Successfully switched to usermode!\n";
    syscall_write(hello);

    init_framebuffer();

    while (1) {};

    /*while (1) {
        char *key = malloc(1);
        syscall_readc(key);
        syscall_writec(key);
    };*/
}