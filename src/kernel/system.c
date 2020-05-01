#include <kernel/timer/timer.h>
#include <kernel/io/io.h>
#include <kernel/graphics/vesa.h>
#include <kernel/lib/string.h>
#include <kernel/lib/stdlib.h>
#include <kernel/memory/paging.h>
#include <kernel/lib/stdio.h>
#include <stdarg.h>

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
	_log(f, RED "PNC: %s - Halting system!" RES, msg);
	printf("[%s] PNC: %s - Halting system!\n\n", f, msg);
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
	debug("Halted.");
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