#include <kernel/syscall/syscall.h>

void test_user() {
    syscall_serial_write("Hello, user world!\n");
}