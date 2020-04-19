#include <kernel/timer/timer.h>
#include <kernel/io/io.h>
#include <kernel/graphics/vesa.h>
#include <kernel/lib/string.h>
#include <kernel/lib/stdlib.h>
#include <kernel/memory/paging.h>
#include <kernel/lib/stdio.h>
#include <stdarg.h>

uint32_t initial_esp;

char *vga_buffer = (char *)0x500;

void vga_clear()
{
	uint16_t *terminal_buffer = (uint16_t *)0xB8000;
	for (size_t y = 0; y < 25; y++)
		for (size_t x = 0; x < 80; x++)
			terminal_buffer[y * 80 + x] = 0 | (uint16_t)0x700;
}

static int line = 0;

void vga_log(char *msg)
{
	if (line == 0)
		vga_clear();
	uint16_t *terminal_buffer = (uint16_t *)0xB8000;
	for (size_t i = 0; i < strlen(msg); i++)
		terminal_buffer[line * 80 + i] = (uint16_t)msg[i] | (uint16_t)0x700;
	log("%s", msg);
	char string[80];
	strcpy(string, "[");
	strcat(string, itoa((int)get_time()));
	strcat(string, "] ");
	strcat(string, "INF: ");
	strcat(string, msg);
	strcat(string, "\n");
	strcat(vga_buffer, string);
	line++;
}

void _debug(const char *f, const char *fmt, ...)
{
	serial_printf(MAG "[%s] " RES, f);
	va_list args;
	va_start(args, fmt);
	serial_vprintf(fmt, args);
	va_end(args);

	serial_put('\n');
}

void _info(const char *f, const char *fmt, ...)
{
	serial_printf(BLU "[%s] " RES, f);
	va_list args;
	va_start(args, fmt);
	serial_vprintf(fmt, args);
	va_end(args);

	serial_put('\n');
}

void _warn(const char *f, const char *fmt, ...)
{
	serial_printf(YEL "[%s] " RES, f);
	va_list args;
	va_start(args, fmt);
	serial_vprintf(fmt, args);
	va_end(args);

	serial_put('\n');
}

void _log(const char *f, const char *fmt, ...)
{
	serial_printf(CYN "[%s] " RES, f);
	va_list args;
	va_start(args, fmt);
	serial_vprintf(fmt, args);
	va_end(args);

	serial_put('\n');
}

const char *random_message[10] = { "Uh... Did I do that?",
				   "Layer 8 problem!",
				   "Oops.",
				   "DON'T PANIC!",
				   "Must be a typo.",
				   "I'm tired of this ;(",
				   "PC LOAD LETTER",
				   "Have you tried turning it off and on again?",
				   "Call 01189998819991197253 pls",
				   "Please fix me!" };

void _panic(const char *f, const char *msg)
{
	cli();
	_log(f, RED "PNC: %s - System halted!" RES, msg);
	printf("[%s] PNC: %s - System halted!\n\n", f, msg);
	printf("> %s", random_message[get_time() % 10]);
	halt_loop();
}

void _assert(const char *f, int x)
{
	if (x == 0) {
		_panic(f, "Assertion failed");
	}
}

void halt_loop()
{
	cli();
loop:
	hlt();
	goto loop;
}

void v86(uint8_t code, regs16_t *regs)
{
	paging_disable();
	int32(code, regs);
	paging_enable();
}