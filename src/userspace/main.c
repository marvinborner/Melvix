#include <syscall.h>
#include <mlibc/stdio.h>
#include <mlibc/stdlib.h>

int32_t starts_with(const char *a, const char *b)
{
    size_t length_pre = strlen(b);
    size_t length_main = strlen(a);
    return length_main < length_pre ? 0 : memcmp(b, a, length_pre) == 0;
}

void user_main()
{
    char text[] = "> Successfully switched to usermode!\n";
    printf(text);

    while (1) {
        char *input = readline();
        if (starts_with(input, "ls")) {
            printf(text);
        }
    };
}