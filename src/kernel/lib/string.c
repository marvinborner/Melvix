#include <stddef.h>
#include <kernel/io/io.h>

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

size_t strcmp (const char * s1, const char * s2) {
    for(; *s1 == *s2; ++s1, ++s2)
        if(*s1 == 0)
            return 0;
    return *(unsigned char *)s1 < *(unsigned char *)s2 ? -1 : 1;
}

void strcat(char *dest, const char *src) {
    size_t s_dest = strlen(dest);
    size_t s_orig = strlen(src);

    for (size_t i = 0; i < s_orig; i++) dest[s_dest + i] = src[i];
    dest[s_dest + s_orig] = 0;
}

void strcpy(char *dest, const char *src) {
    size_t s_orig = strlen(src);

    for (size_t i = 0; i < s_orig; i++) dest[i] = src[i];
    dest[s_orig] = 0;
}
