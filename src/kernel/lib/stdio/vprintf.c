#include <stdarg.h>
#include <stdint.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/string.h>
#include <kernel/lib/stdlib.h>
#include <kernel/memory/kheap.h>

void _writes(const char *data)
{
    for (size_t i = 0; i < strlen(data); i++) writec(data[i]);
}

void vprintf(const char *fmt, va_list args)
{
    uint8_t readyToFormat = 0;

    char buff = 0;

    for (; *fmt; fmt++) {
        if (readyToFormat) {
            if (*fmt == '%') {
                writec('%');
                readyToFormat = 0;
                continue;
            }

            buff = *fmt;
            if (buff == 's') {
                const char *str = va_arg(args, const char*);
                _writes(str);
                readyToFormat = 0;
            } else if (buff == 'x') {
                char *p = htoa((uint32_t) va_arg(args, int));
                _writes(p);
                kfree(p);
                readyToFormat = 0;
            } else if (buff == 'd') {
                char *p = itoa(va_arg(args, int));
                _writes(p);
                kfree(p);
                readyToFormat = 0;
            } else if (buff == 'c') {
                writec((char) va_arg(args, int));
                readyToFormat = 0;
            }
        } else {
            if (*fmt == '%')
                readyToFormat = 1;
            else
                writec(*fmt);
        }
    }
}