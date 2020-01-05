#include <stddef.h>
#include <syscall.h>

void *malloc(size_t count)
{
    return (void *) syscall_alloc(count);
}

void free(void *ptr)
{
    syscall_free((uint32_t) ptr);
}
