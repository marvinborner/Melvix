#include <stdint.h>
#include <mlibc/string.h>
#include <mlibc/stdlib.h>

static const char __HTOA_TABLE[] = "0123456789ABCDEF";

char *htoa(uint32_t n)
{
    char *ret = kmalloc(10);

    int i = 0;
    while (n) {
        ret[i++] = __HTOA_TABLE[n & 0xF];
        n >>= 4;
    }

    if (!i) {
        ret[0] = '0';
        i++;
    }

    for (; i <= 9; i++) ret[i] = 0;

    char *aux = strdup(ret);
    kfree(ret);
    ret = aux;

    strinv(ret);
    return ret;
}