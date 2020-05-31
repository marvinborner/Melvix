#include <stdint.h>
#include <system.h>

u8 inb(u16 port)
{
	u8 value;
	asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
	return value;
}

u16 inw(u16 port)
{
	u16 value;
	asm volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
	return value;
}

u32 inl(u16 port)
{
	u32 value;
	asm volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
	return value;
}

u32 cpu_flags()
{
	u32 flags;
	asm volatile("pushf\n"
		     "pop %0\n"
		     : "=rm"(flags)::"memory");
	return flags;
}

int interrupts_enabled()
{
	return (cpu_flags() & 0x200) == 0x200;
}

void interrupts_print()
{
	if (interrupts_enabled())
		log(GRN "Interrupts are enabled!" RES);
	else
		log(RED "Interrupts are disabled!" RES);
}

void cli()
{
	asm volatile("cli");
}

void sti()
{
	asm volatile("sti");
}

void hlt()
{
	asm volatile("hlt");
}

void outb(u16 port, u8 data)
{
	asm volatile("outb %0, %1" ::"a"(data), "Nd"(port));
}

void outw(u16 port, u16 data)
{
	asm volatile("outw %0, %1" ::"a"(data), "Nd"(port));
}

void outl(u16 port, u32 data)
{
	asm volatile("outl %0, %1" ::"a"(data), "Nd"(port));
}

void serial_install()
{
	outb(0x3f8 + 1, 0x00);
	outb(0x3f8 + 3, 0x80);
	outb(0x3f8 + 0, 0x03);
	outb(0x3f8 + 1, 0x00);
	outb(0x3f8 + 3, 0x03);
	outb(0x3f8 + 2, 0xC7);
	outb(0x3f8 + 4, 0x0B);
	info("Installed serial connection");
}

int is_transmit_empty()
{
	return inb(0x3f8 + 5) & 0x20;
}

void serial_put(char ch)
{
	while (is_transmit_empty() == 0)
		;
	outb(0x3f8, (u8)ch);
}