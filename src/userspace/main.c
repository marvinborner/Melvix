#include <kernel/syscall/syscall.h>

void test_user() {
    asm volatile ("hlt");
    syscall_serial_write("Hello, user world!\n");
}