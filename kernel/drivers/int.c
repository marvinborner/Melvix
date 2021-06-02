// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <def.h>
#include <drivers/int.h>
#include <drivers/pic.h>
#include <drivers/serial.h>

/**
 * IDT
 */

PROTECTED extern u32 int_table[];
PROTECTED static struct idt_entry idt[256] = { 0 };
PROTECTED static struct idt_ptr idt_ptr = { 0 };

CLEAR void idt_install(void)
{
	idt_ptr.size = sizeof(idt) - 1;
	idt_ptr.base = &idt;

	for (u8 i = 0; i < 3; i++)
		idt[i] = IDT_ENTRY(int_table[i], 0x08, INT_GATE);

	idt[3] = IDT_ENTRY(int_table[3], 0x08, INT_TRAP);
	idt[4] = IDT_ENTRY(int_table[4], 0x08, INT_TRAP);

	for (u8 i = 5; i < 48; i++)
		idt[i] = IDT_ENTRY(int_table[i], 0x08, INT_GATE);

	idt[128] = IDT_ENTRY(int_table[48], 0x08, INT_GATE | INT_USER);
	idt[129] = IDT_ENTRY(int_table[49], 0x08, INT_GATE);

	__asm__ volatile("lidt %0" : : "m"(idt_ptr));
}

/**
 * Exception (trap) handling
 */

PROTECTED const char *int_trap_names[32] = {
	"Division By Zero",
	"Debug",
	"Non Maskable Interrupt",
	"Breakpoint",
	"Into Detected Overflow",
	"Out of Bounds",
	"Invalid Opcode",
	"No Coprocessor",

	"Double Fault",
	"Coprocessor Segment Overrun",
	"Bad TSS",
	"Segment Not Present",
	"Stack Fault",
	"General Protection Fault",
	"Page Fault",
	"Unknown Interrupt",

	"Coprocessor Fault",
	"Alignment Check",
	"Machine Check",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",

	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
};

PROTECTED static void (*int_trap_handlers[16])(u32 esp) = { 0 };

CLEAR void int_trap_handler_add(u32 int_no, void (*handler)(u32 esp))
{
	assert(int_no < COUNT(int_trap_handlers));
	int_trap_handlers[int_no] = handler;
}

static void int_trap_handler(struct int_frame *frame)
{
	static u8 faulting = 0;
	faulting++;

	if (faulting == 2) {
		// Fall back to serial driver
		serial_print("Double fault, halting immediatly\n");
		while (1)
			__asm__ volatile("cli\nhlt");
	}

	assert(frame->int_no < COUNT(int_trap_handlers));
	void (*handler)(u32 esp) = int_trap_handlers[frame->int_no];
	if (handler)
		handler((u32)frame);

	printf("%s Exception (code %x) at 0x%x (ring %d), exiting!\n",
	       int_trap_names[frame->int_no], frame->err_code, frame->eip, RING(frame));
	struct proc *proc = proc_current();
	if (proc) {
		printf("\t-> Exception occurred in %s at addr 0x%x (offset 0x%x)\n", proc->name,
		       frame->eip, frame->eip - proc->entry);
		printf("\t\t-> Process: [entry: %x, kstack: %x, esp %x, ustack: %x]\n", proc->entry,
		       proc->stack.kernel, frame->esp, proc->stack.user);
		proc_exit(proc, 1);
		faulting--;
	} else {
		while (1)
			__asm__ volatile("cli\nhlt");
	}
}

/**
 * Event handling
 */

PROTECTED static void (*int_event_handlers[16])(void) = { 0 };

CLEAR void int_event_handler_add(u32 int_no, void (*handler)(void))
{
	assert(int_no < COUNT(int_event_handlers));
	int_event_handlers[int_no] = handler;
}

#include <mm.h>
static u32 int_event_handler(struct int_frame *frame)
{
	u32 int_no = frame->int_no - 32;
	assert(int_no < COUNT(int_event_handlers));
	void (*handler)(void) = int_event_handlers[int_no];
	if (handler)
		handler();

	if (!int_no)
		return scheduler((u32)frame);
	return (u32)frame;
}

/**
 * Special interrupts (e.g. syscall, yield)
 */

PROTECTED static u32 (*int_special_handlers[16])(u32 esp) = { 0 };

CLEAR void int_special_handler_add(u32 int_no, u32 (*handler)(u32 esp))
{
	assert(int_no < COUNT(int_event_handlers));
	int_special_handlers[int_no] = handler;
}

static u32 int_special_handler(struct int_frame *frame)
{
	u32 int_no = frame->int_no - 128;
	assert(int_no < COUNT(int_event_handlers));
	u32 (*handler)(u32 esp) = int_special_handlers[int_no];
	if (handler)
		return handler((u32)frame);
	return (u32)frame;
}

/**
 * Universal handler
 */

u32 int_handler(u32 esp);
u32 int_handler(u32 esp)
{
	struct int_frame *frame = (struct int_frame *)esp;
	if (frame->int_no < 32)
		int_trap_handler(frame);
	else if (frame->int_no < 48)
		esp = int_event_handler(frame);
	else if (frame->int_no >= 128 && frame->int_no < 144)
		esp = int_special_handler(frame);
	else
		panic("Unknown interrupt: %d\n", frame->int_no);

	pic_ack(frame->int_no);
	return esp;
}
