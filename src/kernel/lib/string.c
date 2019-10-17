#include <stddef.h>

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

size_t strcmp(const char *s1, const char *s2) {
    size_t s_a = strlen(s1);
    for (size_t i = 0; i < s_a; i++) if (s1[i] != s2[i]) return 1;
    return 0;
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
