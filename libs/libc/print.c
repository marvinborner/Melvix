// MIT License, Copyright (c) 2020 Marvin Borner

#include <arg.h>
#include <assert.h>
#include <conv.h>
#include <def.h>
#include <mem.h>
#include <str.h>

static void append(char *dest, char *src, int index)
{
	for (u32 i = index; i < strlen(src) + index; i++)
		dest[i] = src[i - index];
	dest[index + strlen(src)] = 0;
}

int vsnprintf(char *str, u32 size, const char *format, va_list ap)
{
	u8 ready_to_format = 0;

	u32 i = 0;
	char buf = 0;

	// TODO: Fix format buffer overflow exploit
	char format_buffer[42] = { 0 };

	for (; *format; format++) {
		if (ready_to_format) {
			ready_to_format = 0;

			if (*format == '%') {
				str[i] = '%';
				continue;
			}

			buf = *format;

			// TODO: Improve this repetitive code
			if (buf == 's') {
				char *string = va_arg(ap, char *);
				assert(string);
				append(str, string, i);
				i = strlen(str);
			} else if (buf == 'x') {
				conv_base(va_arg(ap, u32), format_buffer, 16, 0);
				append(str, format_buffer, i);
				i = strlen(str);
			} else if (buf == 'd' || buf == 'i') {
				conv_base(va_arg(ap, s32), format_buffer, 10, 1);
				append(str, format_buffer, i);
				i = strlen(str);
			} else if (buf == 'u') {
				conv_base(va_arg(ap, u32), format_buffer, 10, 0);
				append(str, format_buffer, i);
				i = strlen(str);
			} else if (buf == 'o') {
				conv_base(va_arg(ap, u32), format_buffer, 8, 0);
				append(str, format_buffer, i);
				i = strlen(str);
			} else if (buf == 'b') {
				conv_base(va_arg(ap, u32), format_buffer, 2, 0);
				append(str, format_buffer, i);
				i = strlen(str);
			} else if (buf == 'c') {
				str[i] = (char)va_arg(ap, int);
				i++;
			} else {
				assert(0);
			}
		} else {
			if (*format == '%')
				ready_to_format = 1;
			else {
				str[i] = *format;
				if (++i == size) {
					str[i] = 0;
					break;
				}
			}
		}

		memset(format_buffer, 0, sizeof(format_buffer));
	}

	return strlen(str);
}

int snprintf(char *str, u32 size, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int len = vsnprintf(str, size, format, ap);
	va_end(ap);

	return len;
}

#ifdef USER

#include <sys.h>
#define PATH_OUT "/proc/self/io/out"
#define PATH_LOG "/proc/self/io/log"
#define PATH_ERR "/proc/self/io/err"

int vprintf(const char *format, va_list ap)
{
	return vfprintf(PATH_OUT, format, ap);
}

int vfprintf(const char *path, const char *format, va_list ap)
{
	char buf[1024] = { 0 };
	int len = vsnprintf(buf, sizeof(buf), format, ap);
	return write(path, buf, 0, len);
}

int fprintf(const char *path, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int len = vfprintf(path, format, ap);
	va_end(ap);

	return len;
}

int printf(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int len = vprintf(format, ap);
	va_end(ap);

	return len;
}

int log(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int len = vfprintf(PATH_LOG, format, ap);
	va_end(ap);

	return len;
}

NORETURN void err(int code, const char *format, ...)
{
	if (errno != EOK)
		log("ERRNO: %d (%s)\n", errno, strerror(errno));
	va_list ap;
	va_start(ap, format);
	vfprintf(PATH_ERR, format, ap);
	va_end(ap);
	exit(code);
}

int print(const char *str)
{
	return write(PATH_OUT, str, 0, strlen(str));
}

#else

// The kernel prints everything into the serial console

#include <cpu.h>
#include <mm.h>
#include <proc.h>
#include <serial.h>

#define RED "\x1B[1;31m"
#define GRN "\x1B[1;32m"
#define YEL "\x1B[1;33m"
#define BLU "\x1B[1;34m"
#define MAG "\x1B[1;35m"
#define CYN "\x1B[1;36m"
#define WHT "\x1B[1;37m"
#define RES "\x1B[0m"

static void print_kernel(const char *str)
{
	serial_print(RED);
	serial_print("[KER] ");
	serial_print(str);
	serial_print(RES);
}

int vprintf(const char *format, va_list ap)
{
	char buf[1024] = { 0 };
	int len = vsnprintf(buf, sizeof(buf), format, ap);
	print_kernel(buf);
	return len;
}

int printf(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int len = vprintf(format, ap);
	va_end(ap);

	return len;
}

int print_app(enum stream_defaults id, const char *proc_name, const char *str)
{
	if (id == STREAM_LOG)
		serial_print(CYN "[LOG] to ");
	else if (id == STREAM_ERR)
		serial_print(YEL "[ERR] to ");
	serial_print(proc_name);
	serial_print(": ");
	stac();
	serial_print(str);
	clac();
	serial_print(RES);
	return 1;
}

int print(const char *str)
{
	print_kernel(str);
	return strlen(str);
}

void print_trace(u32 count)
{
	struct frame {
		struct frame *ebp;
		u32 eip;
	} * stk;
	__asm__ volatile("movl %%ebp, %0;" : "=r"(stk));
	print("EBP\tEIP\n");
	for (u32 i = 0; stk && i < count; i++) {
		printf("0x%x\t0x%x\n", stk->ebp, stk->eip);
		stk = stk->ebp;
	}
}

#endif

NORETURN void panic(const char *format, ...)
{
	char buf[1024] = { 0 };
	va_list ap;
	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);
#ifdef KERNEL
	print("--- DON'T PANIC! ---\n");
	print(buf);
	print_trace(10);
	while (1)
		__asm__ volatile("cli\nhlt");
#else
	err(1, buf);
#endif
}
