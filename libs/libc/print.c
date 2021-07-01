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
	double temp_double;
	vec2 temp_vec2;

	char buffer[64] = { 0 };

	// TODO: Fix potential memory overflows because of str[length++]=xxx
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
			case 'b':
				temp_int = va_arg(ap, int);
				itoa(temp_int, buffer, 2);
				length += strlcpy(&str[length], buffer, size - length);
				break;
			case 'd':
				temp_int = va_arg(ap, int);
				itoa(temp_int, buffer, 10);
				length += strlcpy(&str[length], buffer, size - length);
				break;
			case 'v':
				str[length++] = '(';
				temp_vec2 = va_arg(ap, vec2);
				itoa(temp_vec2.x, buffer, 10);
				length += strlcpy(&str[length], buffer, size - length);
				str[length++] = '/';
				itoa(temp_vec2.y, buffer, 10);
				length += strlcpy(&str[length], buffer, size - length);
				str[length++] = ')';
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
	return vdprintf(DEV_LOGGER, format, ap);
}

int vfprintf(const char *path, const char *format, va_list ap)
{
	char buf[1024] = { 0 };
	int len = vsnprintf(buf, sizeof(buf), format, ap);
	return write(path, buf, 0, len);
}

int vdprintf(enum dev_type io, const char *format, va_list ap)
{
	char buf[1024] = { 0 };
	int len = vsnprintf(buf, sizeof(buf), format, ap);
	return dev_write(io, buf, 0, len);
}

int fprintf(const char *path, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int len = vfprintf(path, format, ap);
	va_end(ap);

	return len;
}

int dprintf(enum dev_type io, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int len = vdprintf(io, format, ap);
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
	int len = vdprintf(DEV_LOGGER, format, ap);
	va_end(ap);

	return len;
}

NORETURN void err(int code, const char *format, ...)
{
	log("Exiting process with code %d\n", code);
	if (errno != EOK)
		log("ERRNO: %d (%s)\n", errno, strerror(errno));
	va_list ap;
	va_start(ap, format);
	vdprintf(DEV_LOGGER, format, ap);
	va_end(ap);
	exit(code);
}

int print(const char *str)
{
	return dev_write(DEV_LOGGER, str, 0, strlen(str));
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

void print_trace_custom(u32 stack, u32 count)
{
	stac();
	struct frame {
		struct frame *ebp;
		u32 eip;
	} *stk = (void *)stack;
	printf("Stack trace for 0x%x\n", stack);
	print("\tEBP\tEIP\n");
	for (u32 i = 0; stk && memory_readable(stk) && i < count; i++) {
		if (!stk->ebp || !stk->eip)
			break;
		printf("\t0x%x\t0x%x\n", stk->ebp, stk->eip);
		stk = stk->ebp;
	}
	print("Stack trace end\n");
	clac();
}

void print_trace(void)
{
	u32 ebp;
	__asm__ volatile("movl %%ebp, %0;" : "=r"(ebp));
	print_trace_custom(ebp, 10);
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
	print_trace();
	while (1)
		__asm__ volatile("cli\nhlt");
#else
	err(1, buf);
#endif
}
