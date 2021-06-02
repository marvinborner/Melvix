// MIT License, Copyright (c) 2020 Marvin Borner

#include <arg.h>
#include <assert.h>
#include <conv.h>
#include <def.h>
#include <mem.h>
#include <str.h>

#define RED "\x1B[1;31m"
#define GRN "\x1B[1;32m"
#define YEL "\x1B[1;33m"
#define BLU "\x1B[1;34m"
#define MAG "\x1B[1;35m"
#define CYN "\x1B[1;36m"
#define WHT "\x1B[1;37m"
#define RES "\x1B[0m"

int vsnprintf(char *str, u32 size, const char *format, va_list ap)
{
	u32 length = 0;

	int temp_int;
	char temp_ch;
	char *temp_str;
	f64 temp_double;

	char buffer[64] = { 0 };

	char ch;
	while ((ch = *format++)) {
		if (ch == '%') {
			switch (*format++) {
			case '%':
				str[length++] = '%';
				break;
			case 'c':
				temp_ch = va_arg(ap, int);
				str[length++] = temp_ch;
				break;
			case 's':
				temp_str = va_arg(ap, char *);
				length += strlcpy(&str[length], temp_str, size - length);
				break;
			case 'd':
				temp_int = va_arg(ap, int);
				itoa(temp_int, buffer, 10);
				length += strlcpy(&str[length], buffer, size - length);
				break;
			case 'x':
				temp_int = va_arg(ap, int);
				itoa(temp_int, buffer, 16);
				length += strlcpy(&str[length], buffer, size - length);
				break;
			case 'f':
				temp_double = va_arg(ap, double);
				ftoa(temp_double, buffer, 5);
				length += strlcpy(&str[length], buffer, size - length);
				break;
			default:
				print("Unknown printf format\n");
			}
		} else {
			str[length++] = ch;
		}
	}

	return length;
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

int vprintf(const char *format, va_list ap)
{
	return viprintf(IO_LOGGER, format, ap);
}

int vfprintf(const char *path, const char *format, va_list ap)
{
	char buf[1024] = { 0 };
	int len = vsnprintf(buf, sizeof(buf), format, ap);
	return write(path, buf, 0, len);
}

int viprintf(enum io_type io, const char *format, va_list ap)
{
	char buf[1024] = { 0 };
	int len = vsnprintf(buf, sizeof(buf), format, ap);
	return io_write(io, buf, 0, len);
}

int fprintf(const char *path, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int len = vfprintf(path, format, ap);
	va_end(ap);

	return len;
}

int iprintf(enum io_type io, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int len = viprintf(io, format, ap);
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
	int len = viprintf(IO_LOGGER, format, ap);
	va_end(ap);

	return len;
}

NORETURN void err(int code, const char *format, ...)
{
	if (errno != EOK)
		log("ERRNO: %d (%s)\n", errno, strerror(errno));
	va_list ap;
	va_start(ap, format);
	viprintf(IO_LOGGER, format, ap);
	va_end(ap);
	exit(code);
}

int print(const char *str)
{
	return io_write(IO_LOGGER, str, 0, strlen(str));
}

#else

// The kernel prints everything into the serial console

#include <drivers/cpu.h>
#include <drivers/serial.h>
#include <mm.h>
#include <proc.h>

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

int print_prefix(void)
{
	char buf[64] = { 0 };
	snprintf(buf, sizeof(buf), CYN "[LOG] to %s (%d): ", proc_current()->name,
		 proc_current()->pid);
	serial_print(buf);
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
	for (u32 i = 0; stk && memory_readable(stk) && i < count; i++) {
		if (!stk->ebp || !stk->eip)
			break;
		printf("0x%x\t0x%x\n", stk->ebp, stk->eip);
		stk = stk->ebp;
	}
}

#endif

void panic(const char *format, ...)
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
