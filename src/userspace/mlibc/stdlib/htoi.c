#include <stddef.h>
#include <mlibc/math.h>
#include <mlibc/stdlib.h>

int htoi(char *str)
{
    size_t s_str = strlen(str);

    size_t i = 0;
    int ret = 0;
    for (; i < s_str; i++) {
        char c = str[i];
        int aux = 0;
        if (c >= '0' && c <= '9')
            aux = c - '0';
        else if (c >= 'A' && c <= 'F')
            aux = (c - 'A') + 10;

        ret += aux * pow(16, (s_str - i) - 1);
    }

    return ret;
}