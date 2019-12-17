#include <syscall.h>
#include <graphics/graphics.h>
#include <mlibc/stdio.h>

void user_main()
{
    char hello[] = "> Successfully switched to usermode!\n";
    syscall_write(hello);

    // init_framebuffer();

    while (1) {
        char key = getch();
        writec(key);
    };
}