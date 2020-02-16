#include <kernel/lib/math.h>
#include <stdint.h>
#include <kernel/lib/string.h>
#include <kernel/memory/alloc.h>
#include <kernel/memory/paging.h>

static const char ITOA_TABLE[] = "0123456789";

char *itoa(int n)
{
    if (paging_enabled == 0)
        return "0"; // kmalloc isn't available

    if (!n) {
        char *ret = (char *) kmalloc(2);
        ret[0] = '0';
        ret[1] = 0;
        return ret;
    }
    uint8_t negative = (uint8_t) (n < 0);
    if (negative) n *= -1;

    int sz;
    for (sz = 0; n % pow(10, sz) != n; sz++) {}

    char *ret = (char *) kmalloc((uint32_t) (sz + 1));

    for (int i = 0; i < sz; i++) {
        int digit = (n % pow(10, i + 1)) / pow(10, i);
        ret[i] = ITOA_TABLE[digit];
    }
    ret[sz] = 0;

    if (negative) {
        char *aux = (char *) kmalloc((uint32_t) (sz + 2));
        strcpy(aux, ret);
        aux[sz] = '-';
        aux[sz + 1] = 0;
        kfree(ret);
        ret = aux;
    }

    strinv(ret);
    return ret;
}